#include "Include/Graphics/ImageBasedLightingRenderer.h"

#include "Include/Core/Log.h"
#include "Include/Graphics/ModelData.h"
#include "Include/Graphics/Shader.h"
#include "Include/Graphics/ModelLoader.h"
#include "Include/Graphics/Utils/KtxImage.h"

#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/core/utils/logger.hpp>
#include <glad/glad.h>

namespace mnemosy::graphics
{
	ImageBasedLightingRenderer::ImageBasedLightingRenderer()
	{
		glGenFramebuffers(1, &m_fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

		m_imagedBasedLightingShader = std::make_unique<Shader>("../Code/Engine/Src/Shaders/imageBasedLighting.vert", "../Code/Engine/Src/Shaders/imageBasedLighting.frag");

		std::unique_ptr<ModelLoader> modelLoader = std::make_unique <ModelLoader>();
		m_unitCube = modelLoader->LoadModelDataFromFile("../Resources/Meshes/skyboxMesh.fbx");

		// shut tf off 
		cv::utils::logging::setLogLevel(cv::utils::logging::LogLevel::LOG_LEVEL_WARNING);

		//RenderBrdfLutTexture(true);
		LoadBrdfLutTextureFromFile(); // Loss in image quality

	}

	ImageBasedLightingRenderer::~ImageBasedLightingRenderer()
	{
		glDeleteFramebuffers(1, &m_fbo);
		m_fbo = 0;
		delete m_unitCube;
		m_unitCube = nullptr;
	}

	void ImageBasedLightingRenderer::RenderEquirectangularToCubemapTexture(unsigned int& cubemapTextureID, unsigned int& equirectangularTextureID, unsigned int textureRes, bool makeConvolutedIrradianceMap)
	{
		glViewport(0, 0, textureRes, textureRes);

		//unsigned int fbo;
		//glGenFramebuffers(1, &fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
		
		for (int i = 0; i < 6; i++)
		{
			int side = i;

			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + side, cubemapTextureID, 0);

			//bool framebufferSuccessfull = glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE;

			if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			{
				MNEMOSY_ASSERT(false, "Framebuffer is not complete");

				//glDeleteFramebuffers(1, &fbo);
				glBindFramebuffer(GL_FRAMEBUFFER, 0);
				
				return;
			}

			glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTextureID);

			glViewport(0, 0, textureRes, textureRes);

			glClearColor(1.0f, 0.0f, 1.0f, 1.f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			m_imagedBasedLightingShader->Use();

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, equirectangularTextureID);


			m_imagedBasedLightingShader->SetUniformInt("_equirectangularMap", 0);
			m_imagedBasedLightingShader->SetUniformInt("_currentFace", side);

			if (makeConvolutedIrradianceMap)
				m_imagedBasedLightingShader->SetUniformInt("_mode", 1); 
			else
				m_imagedBasedLightingShader->SetUniformInt("_mode", 0); 


			DrawIntoFramebuffer();


		}

			// return to default framebuffer
			glBindFramebuffer(GL_FRAMEBUFFER, 0);

	}

	void ImageBasedLightingRenderer::RenderEquirectangularToPrefilteredCubemapTexture(unsigned int& cubemapID, unsigned int& equirectangularTextureID, unsigned int resolution)
	{
		unsigned int textureResolution = resolution;
		glViewport(0, 0, resolution, resolution);

		glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

		m_imagedBasedLightingShader->Use();

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, equirectangularTextureID);

		m_imagedBasedLightingShader->SetUniformInt("_equirectangularMap", 0);
		m_imagedBasedLightingShader->SetUniformInt("_mode", 2);

		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapID);

		unsigned int maxMipLevels = 5;

		for (unsigned int mip = 0; mip < maxMipLevels; ++mip)
		{
			unsigned int mipRes = int(textureResolution * std::pow(0.5, mip));

			glViewport(0, 0, mipRes, mipRes);

			float roughness = float(mip) / float(maxMipLevels - 1);
			m_imagedBasedLightingShader->SetUniformFloat("_roughness", roughness);

			for (int i = 0; i < 6; i++)
			{
				int face = i;

				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, cubemapID, mip);


				glClearColor(1.0f, 0.0f, 1.0f, 1.f);
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

				m_imagedBasedLightingShader->SetUniformInt("_currentFace", face);

				DrawIntoFramebuffer();
			}
		}

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapID);




		// return to default framebuffer
		//glDeleteFramebuffers(1, &fbo);
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
	void ImageBasedLightingRenderer::RenderBrdfLutTexture(bool exportToFile)
	{
		if (m_brdfLutTexture_isGenerated)
		{
			MNEMOSY_INFO("BrdfLutTexture is already genereated")
				return;
		}



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

		if (exportToFile)
		{
			glBindTexture(GL_TEXTURE_2D, m_brdfLutTextureID);

			// TODO: deallocate malloc memory
			float* gl_texture_bytes = (float*)malloc(sizeof(float) * res * res * 3);

			//glgetteximage(gl_texture_2d, 0 /* mipmap level */, gl_bgr, gl_unsigned_byte, gl_texture_bytes);

			glGetTexImage(GL_TEXTURE_2D, 0, GL_BGR, GL_FLOAT, gl_texture_bytes);
			cv::Mat img = cv::Mat(res, res, CV_32FC3, gl_texture_bytes);

			cv::flip(img, img, 0);
			
			cv::imwrite("../Resources/Textures/ibl/ibl_brdfLut.tif", img);
			
			
			//delete gl_texture_bytes;
			img.release();

		}


		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_brdfLutTextureID);
		glGenerateMipmap(GL_TEXTURE_2D);

		//glDeleteFramebuffers(1, &fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		m_brdfLutTexture_isGenerated = true;

		MNEMOSY_DEBUG("Generated brdf lut texture");

	}

	void ImageBasedLightingRenderer::LoadBrdfLutTextureFromFile()
	{

		if (m_brdfLutTexture_isGenerated)
		{

			return;
		}
		
		cv::Mat pic = cv::imread("../Resources/Textures/ibl/ibl_brdfLut.tif", cv::IMREAD_UNCHANGED);

		MNEMOSY_ASSERT(!pic.empty(), "Couldnt load brdf lut texture from filepath: ../Resources/Textures/ibl/ibl_brdfLut.tif");
		
		cv::flip(pic, pic, 0);



		glGenTextures(1, &m_brdfLutTextureID);

		int res = m_brdfLutResolution;
		glBindTexture(GL_TEXTURE_2D, m_brdfLutTextureID);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, pic.cols, pic.rows, 0, GL_BGR, GL_FLOAT, pic.ptr());
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_brdfLutTextureID);
		glGenerateMipmap(GL_TEXTURE_2D);


		pic.release();
		m_brdfLutTexture_isGenerated = true;

		MNEMOSY_DEBUG("Generated brdf lut");



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

} // !mnemosy::graphics