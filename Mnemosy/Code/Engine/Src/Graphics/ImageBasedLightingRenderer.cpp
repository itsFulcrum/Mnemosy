#include "Include/Graphics/ImageBasedLightingRenderer.h"

#include "Include/MnemosyEngine.h"
#include "Include/Core/Log.h"


#include "Include/Core/FileDirectories.h"
#include "Include/Graphics/ModelData.h"
#include "Include/Graphics/Shader.h"
#include "Include/Graphics/ModelLoader.h"
#include "Include/Graphics/Utils/KtxImage.h"

#include <filesystem>
#include <glad/glad.h>


namespace mnemosy::graphics
{

	void ImageBasedLightingRenderer::Init()
	{

		m_fbo = 0;
		m_brdfLutTextureID = 0;
		ModelData* m_unitCube = nullptr;
		m_brdfLutResolution = 512;


		m_framebufferGenerated = false;
		m_brdfLutTexture_isGenerated = false;

		LoadBrdfLutTexture();

	}

	void ImageBasedLightingRenderer::Shutdown() {
		if (m_framebufferGenerated) {
			glDeleteFramebuffers(1, &m_fbo);
			m_fbo = 0;
			m_framebufferGenerated = false;
		}

		delete m_unitCube;
		m_unitCube = nullptr;
	}

	void ImageBasedLightingRenderer::RenderEquirectangularToCubemapTexture(unsigned int& ColorCubemapTextureID, unsigned int& equirectangularTextureID, unsigned int textureRes, bool genMips)
	{
		PrepareCubemapRendering();
		
		glViewport(0, 0, textureRes, textureRes);

		glBindTexture(GL_TEXTURE_CUBE_MAP, ColorCubemapTextureID);
		

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, equirectangularTextureID);
		m_imagedBasedLightingShader->Use();
		m_imagedBasedLightingShader->SetUniformInt("_equirectangularMap", 0);
		m_imagedBasedLightingShader->SetUniformInt("_mode", 0); 

		glBindVertexArray(m_unitCube->meshes[0].vertexArrayObject);
		
		for (int cubeFace = 0; cubeFace < 6; cubeFace++)
		{
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + cubeFace, ColorCubemapTextureID, 0);

			m_imagedBasedLightingShader->SetUniformInt("_currentFace", cubeFace);
			
			// draw call
			glDrawElements(GL_TRIANGLES, GLsizei(m_unitCube->meshes[0].indecies.size()), GL_UNSIGNED_INT, 0);
		}

		glBindVertexArray(0);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, ColorCubemapTextureID);
		if (genMips) {
			glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
		}


		MNEMOSY_DEBUG("Rendered Equirectangular to Cubemap..");

		EndCubemapRendering();
	}

	void ImageBasedLightingRenderer::RenderEquirectangularToIrradianceCubemapTexture( unsigned int& irradianceCubemapTextureID, unsigned int& equirectangularTextureID, unsigned int textureRes, bool genMips)
	{
		PrepareCubemapRendering();

		glViewport(0, 0, textureRes, textureRes);

		glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceCubemapTextureID);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, equirectangularTextureID);
		m_imagedBasedLightingShader->Use();
		m_imagedBasedLightingShader->SetUniformInt("_equirectangularMap", 0);
		m_imagedBasedLightingShader->SetUniformInt("_mode", 1);
		

		// bind vertex buffer
		glBindVertexArray(m_unitCube->meshes[0].vertexArrayObject);

		for (int cubeFace = 0; cubeFace < 6; cubeFace++)
		{
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + cubeFace, irradianceCubemapTextureID, 0);

			/*glViewport(0, 0, textureRes, textureRes);
			glClearColor(1.0f, 0.0f, 1.0f, 1.f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, equirectangularTextureID);*/

			m_imagedBasedLightingShader->SetUniformInt("_currentFace", cubeFace);

			// draw call
			glDrawElements(GL_TRIANGLES, GLsizei(m_unitCube->meshes[0].indecies.size()), GL_UNSIGNED_INT, 0);
		}
		glBindVertexArray(0);


		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceCubemapTextureID);
		glGenerateMipmap(GL_TEXTURE_CUBE_MAP);


		MNEMOSY_DEBUG("Rendered Equirectangular to Irradiance..");

		EndCubemapRendering();
	}

	void ImageBasedLightingRenderer::RenderEquirectangularToPrefilteredCubemapTexture(unsigned int& prefilterCubemapID, unsigned int& equirectangularTextureID, unsigned int resolution) {
		PrepareCubemapRendering();

		// for prefilter gen mips first not after 
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterCubemapID);
		glGenerateMipmap(GL_TEXTURE_CUBE_MAP); // yes gen mipmap here is neccesary

		glViewport(0, 0, resolution, resolution);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, equirectangularTextureID);

		m_imagedBasedLightingShader->Use();
		m_imagedBasedLightingShader->SetUniformInt("_equirectangularMap", 0);
		m_imagedBasedLightingShader->SetUniformInt("_mode", 2);

		glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterCubemapID);

		// bind vertex buffer
		glBindVertexArray(m_unitCube->meshes[0].vertexArrayObject);

		unsigned int maxMipLevels = 8; // is dependent on prefilter resolution of 512
		for (unsigned int mip = 0; mip < maxMipLevels; ++mip)
		{
			unsigned int mipRes = int(resolution * std::pow(0.5, mip));
			glViewport(0, 0, mipRes, mipRes);

			float roughness = float(mip) / float(maxMipLevels - 2); // 6


			m_imagedBasedLightingShader->SetUniformFloat("_roughness", roughness);

			for (int cubeFace = 0; cubeFace < 6; cubeFace++)
			{
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + cubeFace, prefilterCubemapID, mip);
				/*glClearColor(1.0f, 0.0f, 1.0f, 1.f);
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);*/

				m_imagedBasedLightingShader->SetUniformInt("_currentFace", cubeFace);

				// draw call
				glDrawElements(GL_TRIANGLES, GLsizei(m_unitCube->meshes[0].indecies.size()), GL_UNSIGNED_INT, 0);
				//DrawIntoFramebuffer();
			}
		}

		// unbind vertex buffer
		glBindVertexArray(0);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterCubemapID);

		MNEMOSY_DEBUG("Rendered Equirectangular to Prefilter..");

		EndCubemapRendering();
	}

	void ImageBasedLightingRenderer::PrepareCubemapRendering() {

		if (!IsShaderAndMeshInitialized()) {
			InitializeMeshAndShader();
		}
		glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
	}

	void ImageBasedLightingRenderer::EndCubemapRendering() {

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void ImageBasedLightingRenderer::BindBrdfLutTexture(unsigned int location)
	{
		if (m_brdfLutTexture_isGenerated)
		{
			glActiveTexture(GL_TEXTURE0 + location);
			glBindTexture(GL_TEXTURE_2D, m_brdfLutTextureID);

		}
		else {
			MNEMOSY_ERROR("ImageBaseLightingRenderer::BindBrdfLutTexture: brdf lut texture is not yet generated");
		}

	}

	void ImageBasedLightingRenderer::RenderBrdfLutTextureAndSafeKtx(const char* exportpath, bool exportToFile)
	{
		if (m_brdfLutTexture_isGenerated)
		{
			MNEMOSY_INFO("ImageBasedLightingRenderer::RenderBrdfLutTextureAndSafeKtx - BrdfLutTexture is already genereated")
			return;
		}

		if (!IsShaderAndMeshInitialized())
			InitializeMeshAndShader();

		glGenTextures(1, &m_brdfLutTextureID);

		int res = m_brdfLutResolution;
		glBindTexture(GL_TEXTURE_2D, m_brdfLutTextureID);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RG32F, res, res, 0, GL_RG, GL_FLOAT, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_brdfLutTextureID, 0);

		glViewport(0, 0, res, res);

		glClearColor(1.0f, 0.0f, 1.0f, 1.f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		m_imagedBasedLightingShader->Use();
		m_imagedBasedLightingShader->SetUniformInt("_mode", 3);
		DrawIntoFramebuffer();

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_brdfLutTextureID);
		glGenerateMipmap(GL_TEXTURE_2D);

		m_brdfLutTexture_isGenerated = true;


		if (exportToFile)
		{
			glBindTexture(GL_TEXTURE_2D, m_brdfLutTextureID);

			KtxImage brdfLut;
			bool success = brdfLut.SaveBrdfLutKtx(exportpath, m_brdfLutTextureID, m_brdfLutResolution);
			
			if(success)
				MNEMOSY_INFO("Generated brdf lut texture and safed to: {}", exportpath);
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void ImageBasedLightingRenderer::LoadBrdfLutTexture()
	{
		// looking for brdf lut texture file 
		// if found load it if not generate new file and bind it

		if (m_brdfLutTexture_isGenerated)
			return;


		//mnemosy::core::FileDirectories& fd = MnemosyEngine::GetInstance().GetFileDirectories();
		std::filesystem::path pathToFile = MnemosyEngine::GetInstance().GetFileDirectories().GetTexturesPath() / std::filesystem::path("ibl/ibl_brdfLut.ktx2");
		std::filesystem::directory_entry file = std::filesystem::directory_entry(pathToFile);

		bool ktxFileExists = true;
		if (!file.exists())
		{
			MNEMOSY_WARN("ImageBasedLightingRenderer::LoadBrdfLutTexture: brdf lut texture file does not exsist at:\n{} \nGenerating new file...", pathToFile.generic_string());
			ktxFileExists = false;
		}

		// hardcoding because loading from file causes weird artifacts atm
		//ktxFileExists = false;
		
		std::string pathToFileString = pathToFile.generic_string(); // idk have to convert to string first stupid c++
		if (ktxFileExists) 
		{
			KtxImage brdfLut;
			brdfLut.LoadBrdfKTX(pathToFileString.c_str(), m_brdfLutTextureID);
			
			m_brdfLutTexture_isGenerated = true;
			return;
		}
		else // !fileExists 
		{
			RenderBrdfLutTextureAndSafeKtx(pathToFileString.c_str(), true);
			return;
		}

		return;

	}

	// private
	void ImageBasedLightingRenderer::DrawIntoFramebuffer()
	{
		m_imagedBasedLightingShader->Use();
		m_imagedBasedLightingShader->SetUniformMatrix4("_viewMatrix", glm::mat4(0.0f));
		m_imagedBasedLightingShader->SetUniformMatrix4("_projectionMatrix", glm::mat4(0.0f));

		for (unsigned int i = 0; i < m_unitCube->meshes.size(); i++)
		{
			glBindVertexArray(m_unitCube->meshes[i].vertexArrayObject);
			glDrawElements(GL_TRIANGLES, GLsizei(m_unitCube->meshes[i].indecies.size()), GL_UNSIGNED_INT, 0);
			glBindVertexArray(0);
		}

	}

	// TODO: Replace model loader with mesh registry
	void ImageBasedLightingRenderer::InitializeMeshAndShader() {

		core::FileDirectories& fd = MnemosyEngine::GetInstance().GetFileDirectories();

		if (!m_framebufferGenerated) {
			glGenFramebuffers(1, &m_fbo);
			m_framebufferGenerated = true;
		}

		if (m_unitCube == nullptr) {

			std::filesystem::path meshes = fd.GetMeshesPath();
			// this MUST be a simple unit cube mesh otherwise the shader will cause a device lost
			std::string skyboxMeshPath = meshes.generic_string() + "/mnemosy_skybox_generation_mesh.fbx"; 

			std::unique_ptr<ModelLoader> modelLoader = std::make_unique <ModelLoader>();
			m_unitCube = modelLoader->LoadModelDataFromFile(skyboxMeshPath);
		}

		if (m_imagedBasedLightingShader == nullptr) {
			std::filesystem::path shaders = fd.GetShadersPath();
			std::string vertex = shaders.generic_string()	+ "/imageBasedLighting.vert";
			std::string fragment = shaders.generic_string() + "/imageBasedLighting.frag";

			
			m_imagedBasedLightingShader = std::make_unique<Shader>(vertex.c_str(), fragment.c_str());
		}
	}

	bool ImageBasedLightingRenderer::IsShaderAndMeshInitialized() {

		if (!m_framebufferGenerated)
			return false;

		if (m_unitCube == nullptr)
			return false;

		if (m_imagedBasedLightingShader == nullptr)
			return false;

		return true;
	}

} // !mnemosy::graphics