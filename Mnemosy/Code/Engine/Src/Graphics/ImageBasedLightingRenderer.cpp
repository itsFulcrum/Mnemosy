#include "Include/Graphics/ImageBasedLightingRenderer.h"

#include "Include/Core/Log.h"
#include "Include/MnemosyEngine.h"
#include "Include/Core/FileDirectories.h"
#include "Include/Graphics/ModelData.h"
#include "Include/Graphics/Shader.h"
#include "Include/Graphics/ModelLoader.h"
#include "Include/Graphics/Utils/KtxImage.h"

#include <string>
#include <filesystem>
//#include <opencv2/core.hpp>
//#include <opencv2/imgcodecs.hpp>
//#include <opencv2/core/utils/logger.hpp>
#include <glad/glad.h>


namespace mnemosy::graphics
{
	ImageBasedLightingRenderer::ImageBasedLightingRenderer()
	{
		glGenFramebuffers(1, &m_fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
		
		//MNEMOSY_DEBUG("BindFramebuffer ImageBaseLighting: {} ", m_fbo);
		
		m_brdfLutTexture_isGenerated = false;
		LoadBrdfLutTexture();
	}

	ImageBasedLightingRenderer::~ImageBasedLightingRenderer()
	{
		glDeleteFramebuffers(1, &m_fbo);
		m_fbo = 0;
		delete m_unitCube;
		m_unitCube = nullptr;
	}

	void ImageBasedLightingRenderer::RenderEquirectangularToCubemapTexture(unsigned int& ColorCubemapTextureID, unsigned int& equirectangularTextureID, unsigned int textureRes)
	{
		if (!IsShaderAndMeshInitialized())
			InitializeMeshAndShader();


		glViewport(0, 0, textureRes, textureRes);
		glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
		
		glBindTexture(GL_TEXTURE_CUBE_MAP, ColorCubemapTextureID);
		for (int cubeFace = 0; cubeFace < 6; cubeFace++)
		{
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + cubeFace, ColorCubemapTextureID, 0);
			if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			{
				MNEMOSY_ASSERT(false, "Framebuffer is not complete");
				glBindFramebuffer(GL_FRAMEBUFFER, 0);
				return;
			}

			glViewport(0, 0, textureRes, textureRes);
			glClearColor(1.0f, 0.0f, 1.0f, 1.f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, equirectangularTextureID);

			m_imagedBasedLightingShader->Use();
			m_imagedBasedLightingShader->SetUniformInt("_equirectangularMap", 0);
			m_imagedBasedLightingShader->SetUniformInt("_currentFace", cubeFace);
			m_imagedBasedLightingShader->SetUniformInt("_mode", 0); 

			DrawIntoFramebuffer();
		}

		// return to default framebuffer
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

	}

	void ImageBasedLightingRenderer::RenderEquirectangularToIrradianceCubemapTexture(unsigned int& irradianceCubemapTextureID, unsigned int& equirectangularTextureID, unsigned int textureRes)
	{
		if (!IsShaderAndMeshInitialized())
			InitializeMeshAndShader();

		glViewport(0, 0, textureRes, textureRes);
		glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

		glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceCubemapTextureID);
		for (int cubeFace = 0; cubeFace < 6; cubeFace++)
		{
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + cubeFace, irradianceCubemapTextureID, 0);
			if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			{
				MNEMOSY_ASSERT(false, "Framebuffer is not complete");
				glBindFramebuffer(GL_FRAMEBUFFER, 0);
				return;
			}

			glViewport(0, 0, textureRes, textureRes);
			glClearColor(1.0f, 0.0f, 1.0f, 1.f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, equirectangularTextureID);

			m_imagedBasedLightingShader->Use();
			m_imagedBasedLightingShader->SetUniformInt("_equirectangularMap", 0);
			m_imagedBasedLightingShader->SetUniformInt("_currentFace", cubeFace);
			m_imagedBasedLightingShader->SetUniformInt("_mode", 1);

			DrawIntoFramebuffer();
		}
		// return to default framebuffer
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void ImageBasedLightingRenderer::RenderEquirectangularToPrefilteredCubemapTexture(unsigned int& prefilterCubemapID, unsigned int& equirectangularTextureID, unsigned int resolution)
	{
		if (!IsShaderAndMeshInitialized())
			InitializeMeshAndShader();

		glViewport(0, 0, resolution, resolution);
		glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, equirectangularTextureID);

		m_imagedBasedLightingShader->Use();
		m_imagedBasedLightingShader->SetUniformInt("_equirectangularMap", 0);
		m_imagedBasedLightingShader->SetUniformInt("_mode", 2);

		glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterCubemapID);

		unsigned int maxMipLevels = 5;
		for (unsigned int mip = 0; mip < maxMipLevels; ++mip)
		{
			unsigned int mipRes = int(resolution * std::pow(0.5, mip));
			glViewport(0, 0, mipRes, mipRes);

			float roughness = float(mip) / float(maxMipLevels - 1);
			m_imagedBasedLightingShader->SetUniformFloat("_roughness", roughness);

			for (int cubeFace = 0; cubeFace < 6; cubeFace++)
			{
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + cubeFace, prefilterCubemapID, mip);
				glClearColor(1.0f, 0.0f, 1.0f, 1.f);
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

				m_imagedBasedLightingShader->SetUniformInt("_currentFace", cubeFace);

				DrawIntoFramebuffer();
			}
		}

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterCubemapID);
		// bind Default Framebuffer
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}



	void ImageBasedLightingRenderer::BindBrdfLutTexture(unsigned int location)
	{
		if (m_brdfLutTexture_isGenerated)
		{
			glActiveTexture(GL_TEXTURE0 + location);
			glBindTexture(GL_TEXTURE_2D, m_brdfLutTextureID);

		}
		else
		{
			MNEMOSY_ERROR("ImageBaseLightingRenderer::BindBrdfLutTexture: brdf lut texture is not yet generated");
		}

	}
	void ImageBasedLightingRenderer::RenderBrdfLutTexture(bool exportToFile) // depricated use ktx export
	{
		//if (m_brdfLutTexture_isGenerated)
		//{
		//	MNEMOSY_INFO("ImageBasedLightingRenderer::RenderBrdfLutTexture - BrdfLutTexture is already genereated")
		//	return;
		//}

		//if (!IsShaderAndMeshInitialized())
		//	InitializeMeshAndShader();

		//glGenTextures(1, &m_brdfLutTextureID);

		//int res = m_brdfLutResolution;
		//glBindTexture(GL_TEXTURE_2D, m_brdfLutTextureID);
		//glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, res, res, 0, GL_RG, GL_FLOAT, 0);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		//glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
		//glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_brdfLutTextureID, 0);

		//glViewport(0, 0, res, res);

		//glClearColor(1.0f, 0.0f, 1.0f, 1.f);
		//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//m_imagedBasedLightingShader->Use();

		//m_imagedBasedLightingShader->SetUniformInt("_mode", 3);

		//DrawIntoFramebuffer();

		//if (exportToFile)
		//{
		//	glBindTexture(GL_TEXTURE_2D, m_brdfLutTextureID);

		//	// TODO: deallocate malloc memory
		//	float* gl_texture_bytes = (float*)malloc(sizeof(float) * res * res * 3);

		//	//glgetteximage(gl_texture_2d, 0 /* mipmap level */, gl_bgr, gl_unsigned_byte, gl_texture_bytes);

		//	glGetTexImage(GL_TEXTURE_2D, 0, GL_BGR, GL_FLOAT, gl_texture_bytes);
		//	cv::Mat img = cv::Mat(res, res, CV_32FC3, gl_texture_bytes);

		//	cv::flip(img, img, 0);
		//	
		//	cv::imwrite("../Resources/Textures/ibl/ibl_brdfLut.tif", img);
		//	
		//	
		//	//delete gl_texture_bytes;
		//	img.release();

		//}


		//glActiveTexture(GL_TEXTURE0);
		//glBindTexture(GL_TEXTURE_2D, m_brdfLutTextureID);
		//glGenerateMipmap(GL_TEXTURE_2D);

		////glDeleteFramebuffers(1, &fbo);
		//glBindFramebuffer(GL_FRAMEBUFFER, 0);

		//m_brdfLutTexture_isGenerated = true;

		//MNEMOSY_DEBUG("Generated brdf lut texture");

	}

	void ImageBasedLightingRenderer::RenderBrdfLutTextureAndSafeKtx(const char* exportpath, bool exportToFile)
	{
		if (m_brdfLutTexture_isGenerated)
		{
			MNEMOSY_INFO("ImageBasedLightingRenderer::RenderBrdfLutTexture - BrdfLutTexture is already genereated")
			return;
		}

		if (!IsShaderAndMeshInitialized())
			InitializeMeshAndShader();

		glGenTextures(1, &m_brdfLutTextureID);

		int res = m_brdfLutResolution;
		glBindTexture(GL_TEXTURE_2D, m_brdfLutTextureID);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, res, res, 0, GL_RG, GL_FLOAT, 0);
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
		MNEMOSY_DEBUG("Generated brdf lut texture");

		if (exportToFile)
		{
			glBindTexture(GL_TEXTURE_2D, m_brdfLutTextureID);

			KtxImage brdfLut;
			brdfLut.SaveBrdfLutKtx(exportpath, m_brdfLutTextureID, m_brdfLutResolution);
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void ImageBasedLightingRenderer::LoadBrdfLutTexture()
	{
		// looking for brdf lut texture file 
		// if found load it if not generate new file and bind it

		if (m_brdfLutTexture_isGenerated)
			return;


		mnemosy::core::FileDirectories& fd = MnemosyEngine::GetInstance().GetFileDirectories();
		std::filesystem::path pathToFile = fd.GetTexturesPath() / std::filesystem::path("ibl/ibl_brdfLut.ktx2");
		std::filesystem::directory_entry file = std::filesystem::directory_entry(pathToFile);

		//MNEMOSY_DEBUG("pathToFile: {}", pathToFile.generic_string());
		bool fileExists = true;
		if (!file.exists())
		{
			MNEMOSY_WARN("ImageBasedLightingRenderer::LoadBrdfLutTexture: brdf lut texture file does not exsist at:\n{} \nGenerating new file...", pathToFile.generic_string());
			fileExists = false;
		}

		std::string p = pathToFile.generic_string(); // idk have to convert to string first stupid c++
		if (fileExists) 
		{

			KtxImage brdfLut;

			brdfLut.LoadKtx(p.c_str(), m_brdfLutTextureID);

			m_brdfLutTexture_isGenerated = true;
			return;
		}
		else // !fileExists 
		{
			RenderBrdfLutTextureAndSafeKtx(p.c_str(), true);
			return;
		}


		// Depricated way of loading brdfLut. - not using below
		{
		//cv::Mat pic = cv::imread("../Resources/Textures/ibl/ibl_brdfLut.tif", cv::IMREAD_UNCHANGED);

		//MNEMOSY_ASSERT(!pic.empty(), "Couldnt load brdf lut texture from filepath: ../Resources/Textures/ibl/ibl_brdfLut.tif");
		//
		//cv::flip(pic, pic, 0);



		//glGenTextures(1, &m_brdfLutTextureID);

		//int res = m_brdfLutResolution;
		//glBindTexture(GL_TEXTURE_2D, m_brdfLutTextureID);
		//glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, pic.cols, pic.rows, 0, GL_BGR, GL_FLOAT, pic.ptr());
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		//glActiveTexture(GL_TEXTURE0);
		//glBindTexture(GL_TEXTURE_2D, m_brdfLutTextureID);
		//glGenerateMipmap(GL_TEXTURE_2D);


		//pic.release();
		//m_brdfLutTexture_isGenerated = true;

		//MNEMOSY_DEBUG("Loaded brdf lut");
		}

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

	void ImageBasedLightingRenderer::InitializeMeshAndShader()
	{

		if (m_unitCube == nullptr) {

			std::unique_ptr<ModelLoader> modelLoader = std::make_unique <ModelLoader>();
			m_unitCube = modelLoader->LoadModelDataFromFile("../Resources/Meshes/skyboxMesh.fbx");
		}

		if (m_imagedBasedLightingShader == nullptr) 
		{
			m_imagedBasedLightingShader = std::make_unique<Shader>("../Code/Engine/Src/Shaders/imageBasedLighting.vert", "../Code/Engine/Src/Shaders/imageBasedLighting.frag");
		}
	}

	bool ImageBasedLightingRenderer::IsShaderAndMeshInitialized()
	{

		if (m_unitCube == nullptr)
			return false;

		if (m_imagedBasedLightingShader == nullptr)
			return false;

		return true;
	}

} // !mnemosy::graphics