#include "Include/Systems/TextureGenerationManager.h"

#include "Include/MnemosyEngine.h"
#include "Include/Core/FileDirectories.h"
#include "Include/Core/Log.h"

#include "Include/Systems/ExportManager.h"

#include "Include/Graphics/TextureDefinitions.h"
#include "Include/Graphics/Utils/KtxImage.h"
#include "Include/Graphics/Texture.h"
#include "Include/Graphics/Material.h"
#include "Include/Graphics/Shader.h"

#include <glad/glad.h>

namespace mnemosy::systems
{	
	// public methods

	TextureGenerationManager::TextureGenerationManager() {

	}

	TextureGenerationManager::~TextureGenerationManager() {
		
		if (m_pTextureGenShader == nullptr) {
			delete m_pTextureGenShader;
			m_pTextureGenShader = nullptr;
		}

		if (m_VBO != 0) {
			glDeleteBuffers(1, &m_VBO);
			m_VBO = 0;
		}

		if (m_VAO != 0) {
			glDeleteVertexArrays(1, &m_VAO);
			m_VAO = 0;
		}

		if (m_FBO != 0) {
			glDeleteFramebuffers(1,&m_FBO);
			m_FBO = 0;
		}

		if (m_renderTexture_ID != 0) {
			glDeleteTextures(1, &m_renderTexture_ID);
			m_renderTexture_ID = 0;
		}
	}
	
	void TextureGenerationManager::FlipNormalMap(const char* exportPath,graphics::Material& material, bool exportTexture) {
		
		if (!material.isNormalAssigned()) // should never happen but lets be save
			return;

		if (!IsInitialized()) {
			InitializeShaderTextureAndFBO(1024,1024);
		}

		unsigned int width = material.GetNormalTexture().GetWidth();
		unsigned int height = material.GetNormalTexture().GetHeight();
		

		// === START FRAME
		glViewport(0,0,width, height);

		// Bind framebuffer
		glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);
		// Resize render texture
		glBindTexture(GL_TEXTURE_2D, m_renderTexture_ID);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, width, height, 0, GL_RGB, GL_FLOAT, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// clear frame
		glClearColor(0.0f, 0.0f, 0.0f,1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		// Setup Shader
		m_pTextureGenShader->Use();
		m_pTextureGenShader->SetUniformInt("_mode", 0); // _mode 0 is for filpping normal y channel

		material.GetNormalTexture().BindToLocation(0);
		m_pTextureGenShader->SetUniformInt("_texture0", 0);

		// DRAW CALL
		// render screen quad with shader into render texture
		glBindVertexArray(m_VAO);		
		glDrawArrays(GL_TRIANGLES, 0, 6); // 6 vertecies
		glBindVertexArray(0);

		// Generate MipMaps
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_renderTexture_ID);
		glGenerateMipmap(GL_TEXTURE_2D);


		
		// Export to png or tiff file
		if (exportTexture) {

			systems::ExportManager& exporter = MnemosyEngine::GetInstance().GetExportManager();
			fs::path p = fs::path(exportPath);

			// normal maps should always have RGB channels
			graphics::TextureFormat channelFormat = graphics::TexUtil::get_channel_textureFormat(material.GetNormalTexture().GetTextureFormat());
			graphics::TextureFormat format = graphics::TextureFormat::MNSY_RGB8;
			if (channelFormat == graphics::TextureFormat::MNSY_R16) {
				format = graphics::TextureFormat::MNSY_RGB16;
			}
			else if (channelFormat == graphics::TextureFormat::MNSY_R32) {
				format = graphics::TextureFormat::MNSY_RGB32;
			}


			TextureExportInfo info = TextureExportInfo(p, width, height, format,false);
			exporter.GLTextureExport(m_renderTexture_ID, info,graphics::PBRTextureType::MNSY_TEXTURE_NORMAL);
		}

		// === END FRAME
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	void TextureGenerationManager::InvertRoughness(graphics::Material& material, const char* exportPath, bool exportTexture) {		
		
		MNEMOSY_ASSERT(material.isRoughnessAssigned(), "Check before calling this function if material has a roughness map");

		unsigned int width	= material.GetRoughnessTexture().GetWidth();
		unsigned int height = material.GetRoughnessTexture().GetHeight();

		if (!IsInitialized()) {
			InitializeShaderTextureAndFBO(1024, 1024);
		}

		// === START FRAME
		glViewport(0, 0, width, height);
		// Bind framebuffer
		glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);
		// Resize render texture
		glBindTexture(GL_TEXTURE_2D, m_renderTexture_ID);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, width, height, 0, GL_RGB, GL_FLOAT, nullptr); 


		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// clear frame
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);


		// Setup Shader
		m_pTextureGenShader->Use();
		m_pTextureGenShader->SetUniformInt("_mode", 1); // _mode 1 is for inverting roughness

		material.GetRoughnessTexture().BindToLocation(0);
		m_pTextureGenShader->SetUniformInt("_texture0", 0);

		// DRAW CALL
		// render screen quad with shader into render texture
		glBindVertexArray(m_VAO);
		glDrawArrays(GL_TRIANGLES, 0, 6); // 6 vertecies
		glBindVertexArray(0);

		// Generate MipMaps
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_renderTexture_ID);
		glGenerateMipmap(GL_TEXTURE_2D);


		// Export to png or tiff file
		if (exportTexture) {

			systems::ExportManager& exporter = MnemosyEngine::GetInstance().GetExportManager();
			
			graphics::TextureFormat format = graphics::TexUtil::get_channel_textureFormat(material.GetRoughnessTexture().GetTextureFormat());

			std::filesystem::path p = { exportPath };
			TextureExportInfo info = TextureExportInfo(p,width,height,format,false);

			exporter.GLTextureExport(m_renderTexture_ID,info,graphics::PBRTextureType::MNSY_TEXTURE_ROUGHNESS);
		}

		// === END FRAME
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glBindTexture(GL_TEXTURE_2D, 0);

	}

	void TextureGenerationManager::GenerateOpacityFromAlbedoAlpha(graphics::Material& material, const char* exportPath, bool exportTexture)
	{
		MNEMOSY_ASSERT(material.isAlbedoAssigned(), "Check before calling this function if material has a albedo map");

		unsigned int width  = material.GetAlbedoTexture().GetWidth();
		unsigned int height = material.GetAlbedoTexture().GetHeight();

		if (!IsInitialized()) {
			InitializeShaderTextureAndFBO(1024, 1024);
		}


		// === START FRAME
		glViewport(0, 0, width, height);
		// Bind framebuffer
		glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);
		// Resize render texture
		glBindTexture(GL_TEXTURE_2D, m_renderTexture_ID);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_R16, width, height, 0, GL_RGBA, GL_UNSIGNED_SHORT, nullptr);


		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// clear frame
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);


		// Setup Shader
		m_pTextureGenShader->Use();
		m_pTextureGenShader->SetUniformInt("_mode", 2); // _mode 0 is for filpping normal y channel

		material.GetAlbedoTexture().BindToLocation(0);
		m_pTextureGenShader->SetUniformInt("_texture0", 0);

		// DRAW CALL
		DrawQuad();

		// Export to png or tiff file
		if (exportTexture) {

			systems::ExportManager& exporter = MnemosyEngine::GetInstance().GetExportManager();

			graphics::TextureFormat channelFormat = graphics::TexUtil::get_channel_textureFormat( material.GetAlbedoTexture().GetTextureFormat());


			std::filesystem::path p = { exportPath };
			TextureExportInfo info = TextureExportInfo(p, width, height, channelFormat,false);
			exporter.GLTextureExport(m_renderTexture_ID,info,graphics::PBRTextureType::MNSY_TEXTURE_OPACITY);
		}

		// === END FRAME
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glBindTexture(GL_TEXTURE_2D, 0);

	}

	bool TextureGenerationManager::GenerateChannelPackedTexture(graphics::Material& material, const char* exportPath, bool exportTexture, graphics::ChannelPackType packType, graphics::ChannelPackComponent packComponent_R, graphics::ChannelPackComponent packComponent_G, graphics::ChannelPackComponent packComponent_B, graphics::ChannelPackComponent packComponent_A, unsigned int width, unsigned int height)
	{

		if (!IsInitialized()) {
			InitializeShaderTextureAndFBO(1024, 1024);
		}


		// === START FRAME
		glViewport(0, 0, width, height);
		// Bind framebuffer
		glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);
		// Resize render texture
		glBindTexture(GL_TEXTURE_2D, m_renderTexture_ID);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, nullptr);


		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// clear frame
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		// Setup Shader
		m_pTextureGenShader->Use();
		m_pTextureGenShader->SetUniformInt("_mode", 3); // _mode 3 is for generating channel packed textures


		// Setup texture samplers
		{
			// sampler R channel
			bool channel_r_assignSampler = true;
			bool channel_r_isSingleChannel = true;

			if ((int)packComponent_R < 10) {
				channel_r_isSingleChannel = false;
			}

			graphics::Texture* channel_r_tex = material.GetTextureFromPackComponent(packComponent_R);

			if (channel_r_tex == nullptr) {

				std::string packComponentString = graphics::TexUtil::get_string_from_channelPackComponent(packComponent_R);
				MNEMOSY_WARN("ChannelPacking: Texture for pack type {} is not assigned, result may not be as expected", packComponentString);
				channel_r_assignSampler = false;
			}

			m_pTextureGenShader->SetUniformBool("_channel_r_isAssigned", channel_r_assignSampler);
			m_pTextureGenShader->SetUniformBool("_channel_r_isSingleChannel", channel_r_isSingleChannel);

			if (channel_r_assignSampler) {
				channel_r_tex->BindToLocation(1);
				m_pTextureGenShader->SetUniformInt("_channel_r", 1);
			}
			else {
				float defaultValue = material.GetDefaultValueFromPackComponent(packComponent_R);
				m_pTextureGenShader->SetUniformFloat("_channel_r_defaultValue", defaultValue);
			}

			if (packComponent_R == graphics::MNSY_PACKCOMPONENT_SMOOTHNESS) {
				m_pTextureGenShader->SetUniformBool("_channel_r_invert", true);
			}
			else {
				m_pTextureGenShader->SetUniformBool("_channel_r_invert", false);
			}



			// sampler G channel
			bool channel_g_assignSampler = true;
			bool channel_g_isSingleChannel = true;

			if ((int)packComponent_G < 10) {
				channel_g_isSingleChannel = false;
			}

			graphics::Texture* channel_g_tex = material.GetTextureFromPackComponent(packComponent_G);

			if (channel_g_tex == nullptr) {
				std::string packComponentString = graphics::TexUtil::get_string_from_channelPackComponent(packComponent_G);
				MNEMOSY_WARN("ChannelPacking: Texture for pack type {} is not assigned, result may not be as expected", packComponentString);
				channel_g_assignSampler = false;
			}

			m_pTextureGenShader->SetUniformBool("_channel_g_isAssigned", channel_g_assignSampler);
			m_pTextureGenShader->SetUniformBool("_channel_g_isSingleChannel", channel_g_isSingleChannel);

			if (channel_g_assignSampler) {
				channel_g_tex->BindToLocation(2);
				m_pTextureGenShader->SetUniformInt("_channel_g", 2);
			}
			else {
				float defaultValue = material.GetDefaultValueFromPackComponent(packComponent_G);
				m_pTextureGenShader->SetUniformFloat("_channel_g_defaultValue", defaultValue);
			}


			if (packComponent_G == graphics::MNSY_PACKCOMPONENT_SMOOTHNESS) {
				m_pTextureGenShader->SetUniformBool("_channel_g_invert", true);
			}
			else {
				m_pTextureGenShader->SetUniformBool("_channel_g_invert", false);
			}


			// sampler B channel
			bool channel_b_assignSampler = true;
			bool channel_b_isSingleChannel = true;

			if ((int)packComponent_B < 10) {
				channel_b_isSingleChannel = false;
			}

			graphics::Texture* channel_b_tex = material.GetTextureFromPackComponent(packComponent_B);

			if (channel_b_tex == nullptr) {
				std::string packComponentString = graphics::TexUtil::get_string_from_channelPackComponent(packComponent_B);
				MNEMOSY_WARN("ChannelPacking: Texture for pack type {} is not assigned, result may not be as expected", packComponentString);
				channel_b_assignSampler = false;
			}

			m_pTextureGenShader->SetUniformBool("_channel_b_isAssigned", channel_b_assignSampler);
			m_pTextureGenShader->SetUniformBool("_channel_b_isSingleChannel", channel_b_isSingleChannel);

			if (channel_b_assignSampler) {
				channel_b_tex->BindToLocation(3);
				m_pTextureGenShader->SetUniformInt("_channel_b", 3);
			}
			else {
				float defaultValue = material.GetDefaultValueFromPackComponent(packComponent_B);
				m_pTextureGenShader->SetUniformFloat("_channel_b_defaultValue", defaultValue);
			}

			if (packComponent_B == graphics::MNSY_PACKCOMPONENT_SMOOTHNESS) {
				m_pTextureGenShader->SetUniformBool("_channel_b_invert", true);
			}
			else {
				m_pTextureGenShader->SetUniformBool("_channel_b_invert", false);
			}

			// sampler A channel  // only when pack type is MNSY_PACKTYPE_RGBA

			if (packType == graphics::MNSY_PACKTYPE_RGBA) {
			
				bool channel_a_assignSampler = true;
				bool channel_a_isSingleChannel = true;

				if ((int)packComponent_A < 10) {
					channel_a_isSingleChannel = false;
				}

				graphics::Texture* channel_a_tex = material.GetTextureFromPackComponent(packComponent_A);

				if (channel_a_tex == nullptr) {
					std::string packComponentString = graphics::TexUtil::get_string_from_channelPackComponent(packComponent_A);
					MNEMOSY_WARN("ChannelPacking: Texture for pack type {} is not assigned, result may not be as expected", packComponentString);
					channel_a_assignSampler = false;
				}

				m_pTextureGenShader->SetUniformBool("_channel_a_isAssigned", channel_a_assignSampler);
				m_pTextureGenShader->SetUniformBool("_channel_a_isSingleChannel", channel_a_isSingleChannel);

				if (channel_a_assignSampler) {
					channel_a_tex->BindToLocation(4);
					m_pTextureGenShader->SetUniformInt("_channel_a", 4);
				}
				else {
					float defaultValue = material.GetDefaultValueFromPackComponent(packComponent_A);
					m_pTextureGenShader->SetUniformFloat("_channel_a_defaultValue", defaultValue);
				}

				if (packComponent_A == graphics::MNSY_PACKCOMPONENT_SMOOTHNESS) {
					m_pTextureGenShader->SetUniformBool("_channel_a_invert", true);
				}
				else {
					m_pTextureGenShader->SetUniformBool("_channel_a_invert", false);
				}
			}
			else {

				m_pTextureGenShader->SetUniformBool("_channel_a_isAssigned", false);
			}
		}

		// DRAW CALL
		DrawQuad();

		// Export to png or tiff file
		if (exportTexture) {

			systems::ExportManager& exporter = MnemosyEngine::GetInstance().GetExportManager();

			graphics::TextureFormat Format = graphics::MNSY_RGBA16;

			if (packType == graphics::MNSY_PACKTYPE_RGB) {
				Format = graphics::MNSY_RGB16;
			}

			std::filesystem::path p = { exportPath };
			TextureExportInfo info = TextureExportInfo(p, width, height, Format,false);

			exporter.GLTextureExport(m_renderTexture_ID,info,graphics::PBRTextureType::MNSY_TEXTURE_NONE);
		}


		// === END FRAME
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glBindTexture(GL_TEXTURE_2D, 0);

		return true; // everything succeeded
	}

	bool TextureGenerationManager::IsInitialized() {

		if (m_pTextureGenShader == nullptr)
			return false;

		if (m_VBO == 0)
			return false;

		if (m_VAO == 0)
			return false;

		if (m_FBO == 0)
			return false;

		if (m_renderTexture_ID == 0)
			return false;


		return true;
	}

	void TextureGenerationManager::InitializeShaderTextureAndFBO(unsigned int width, unsigned int height) {
		
		// Load Shader
		if (m_pTextureGenShader == nullptr) {
			fs::path shadersPath = MnemosyEngine::GetInstance().GetFileDirectories().GetShadersPath();
			fs::path vertPath = shadersPath / fs::path("textureGeneration.vert");
			fs::path fragPath = shadersPath / fs::path("textureGeneration.frag");
			m_pTextureGenShader = new graphics::Shader(vertPath.generic_string().c_str(), fragPath.generic_string().c_str());
		}

		// Init Mesh buffers
		if (m_VBO == 0) {
			glGenBuffers(1, &m_VBO);
			glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
		}

		if (m_VAO == 0) {
			glGenVertexArrays(1, &m_VAO);
			glBindVertexArray(m_VAO);

			glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
			glBufferData(GL_ARRAY_BUFFER, sizeof(m_screenQuadVertices), m_screenQuadVertices, GL_STATIC_DRAW);

			// Setup Attributes
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
			
		}

		// Gen Framebuffer
		if (m_FBO == 0) {
			glGenFramebuffers(1, &m_FBO);
			glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);
		}

		// Gen render texture
		if (m_renderTexture_ID == 0) {

			glGenTextures(1, &m_renderTexture_ID);
			glBindTexture(GL_TEXTURE_2D, m_renderTexture_ID);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, width, height, 0, GL_RGB, GL_FLOAT, nullptr);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		}
		
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_renderTexture_ID, 0);
		// Check if frambuffer is complete
		MNEMOSY_ASSERT(glad_glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "Faild to complete framebuffer");
				
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glBindTexture(GL_TEXTURE_2D, 0);

	}

	void TextureGenerationManager::DrawQuad() {

		// DRAW CALL
		// render screen quad with shader into render texture
		glBindVertexArray(m_VAO);
		glDrawArrays(GL_TRIANGLES, 0, 6); // 6 vertecies
		glBindVertexArray(0);

		// Generate MipMaps
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_renderTexture_ID);
		glGenerateMipmap(GL_TEXTURE_2D);

	}

	// private methods



} // !mnemosy::systems