#include "Engine/Include/Graphics/Cubemap.h"
#include "Engine/Include/Core/Log.h"

#include <glad/glad.h>
#include "Engine/Include/MnemosyEngine.h"
#include "Engine/Include/Graphics/ImageBasedLightingRenderer.h"
#include "Engine/Include/Graphics/Image.h"

#include "Engine/Include/Graphics/Utils/KtxImage.h"

#include <memory>

namespace mnemosy::graphics
{
	Cubemap::Cubemap()
	{
	}

	Cubemap::~Cubemap()
	{
		if(m_equirectangularTexture_isGenerated)
			glDeleteTextures(1,&m_equirectangularTextureID);
		if(m_colorCubemap_isGenerated)
			glDeleteTextures(1,&m_colorCubemapID);
		if(m_irradianceMap_isGenerated)
			glDeleteTextures(1,&m_irradianceMapID);
		if(m_prefilterMap_isGenerated)
			glDeleteTextures(1,&m_prefilterMapID);
	}

	void Cubemap::LoadEquirectangularFromFile(const char* path, unsigned int resolution, bool generateConvolutionMaps)
	{
		std::unique_ptr<Image>  img = std::make_unique<Image>();

		bool successfull = img->LoadImageFromFileFLOAT(path, false);
		
		if (!successfull)
		{
			MNEMOSY_ERROR("Failed to load image from file: {}", path);
			//img->FreeData();
			return;
		}

		if (m_equirectangularTexture_isGenerated)
		{
			glDeleteTextures(1, &m_equirectangularTextureID);
			m_equirectangularTexture_isGenerated = false;
		}

		glGenTextures(1, &m_equirectangularTextureID);
		glBindTexture(GL_TEXTURE_2D, m_equirectangularTextureID);
		
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, img->width, img->height, 0, GL_RGB, GL_FLOAT, img->imageDataFLOAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glGenerateMipmap(GL_TEXTURE_2D);

		m_equirectangularTexture_isGenerated = true;
		
		//img->FreeData();


		if (m_colorCubemap_isGenerated)
		{
			glDeleteTextures(1, &m_colorCubemapID);
			m_colorCubemap_isGenerated = false;
		}
		


		if (m_loadCubemapsFromFile) 
		{
			// color Cubemap
			{
				glGenTextures(1, &m_colorCubemapID);
				glBindTexture(GL_TEXTURE_CUBE_MAP, m_colorCubemapID);

				KtxImage ktxImg;
				ktxImg.LoadKtx("../Resources/Textures/Cubemaps/spruitSunrise_cube_color.ktx2", m_colorCubemapID);

				//glBindTexture(GL_TEXTURE_CUBE_MAP, m_colorCubemapID);
				glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
				glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
			}
			m_colorCubemap_isGenerated = true;
			MNEMOSY_DEBUG("LOADED Color cubemap");
			// irradiance cubemap
			{
				glGenTextures(1, &m_irradianceMapID);
				glBindTexture(GL_TEXTURE_CUBE_MAP, m_irradianceMapID);

				KtxImage ktxImg;
				ktxImg.LoadKtx("../Resources/Textures/Cubemaps/spruitSunrise_cube_irradiance.ktx2", m_irradianceMapID);

				//glBindTexture(GL_TEXTURE_CUBE_MAP, m_irradianceMapID);
				glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
				glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
			}
			m_irradianceMap_isGenerated = true;
			MNEMOSY_DEBUG("LOADED irradiance cubemap");
			
			
			// Prefilter cubemap
			{
				glGenTextures(1, &m_prefilterMapID);
				glBindTexture(GL_TEXTURE_CUBE_MAP, m_prefilterMapID);

				KtxImage ktxImg;
				ktxImg.LoadKtx("../Resources/Textures/Cubemaps/spruitSunrise_cube_prefilter.ktx2", m_prefilterMapID);

				//glBindTexture(GL_TEXTURE_CUBE_MAP, m_irradianceMapID);
				glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
				glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);


			}
			m_prefilterMap_isGenerated = true;
			MNEMOSY_DEBUG("LOADED Prefilter cubemap");
		}
		else
		{

			// just from texture to cubemap
			equirectangularToCubemap(m_colorCubemapID, resolution, false);
			m_colorCubemap_isGenerated = true;
			MNEMOSY_DEBUG("Generated cubemap from equirectangular texture");


			if (generateConvolutionMaps)
			{
				// irradiance map
				if (m_irradianceMap_isGenerated)
				{
					glDeleteTextures(1, &m_irradianceMapID);
				}
				equirectangularToCubemap(m_irradianceMapID, m_irradianceMapResolution, true);
				m_irradianceMap_isGenerated = true;
				MNEMOSY_DEBUG("Generated irradiance cubemap");
			
				// prefilter Map
				if (m_prefilterMap_isGenerated)
				{
					glDeleteTextures(1, &m_prefilterMapID);
				}
				equirectangularToPrefilteredCubemap(m_prefilterMapID, m_prefilteredMapResolution);
				m_prefilterMap_isGenerated = true;
				MNEMOSY_DEBUG("Generated prefilterd cubemap");
			}
		}



	}

	void Cubemap::BindColorCubemap(unsigned int location)
	{
		if (m_colorCubemap_isGenerated)
		{
			//MNEMOSY_TRACE("Binding Color Cubemap to location: {} ", location);
			glActiveTexture(GL_TEXTURE0 + location);
			glBindTexture(GL_TEXTURE_CUBE_MAP, m_colorCubemapID);

		}
		else
		{
			MNEMOSY_ERROR("Cubemap::BindColorCubemap: color cubemap is not yet generated");
		}
	}
	void Cubemap::BindIrradianceCubemap(unsigned int location)
	{
		if (m_irradianceMap_isGenerated)
		{
			glActiveTexture(GL_TEXTURE0 + location);
			glBindTexture(GL_TEXTURE_CUBE_MAP, m_irradianceMapID);

		}
		else
		{
			MNEMOSY_ERROR("Cubemap::BindIrradianceCubemap: irradiance map is not yet generated");
		}
	}
	void Cubemap::BindPrefilteredCubemap(unsigned int location)
	{
		if (m_prefilterMap_isGenerated)
		{
			glActiveTexture(GL_TEXTURE0 + location);
			glBindTexture(GL_TEXTURE_CUBE_MAP, m_prefilterMapID);

		}
		else
		{
			MNEMOSY_ERROR("Cubemap::BindPrefilteredCubemap: prefilter map is not yet generated");
		}
	}


// private
	void Cubemap::equirectangularToCubemap(unsigned int& cubemapID, unsigned int resolution, bool makeConvolutedIrradianceMap)
	{
		// create cubemap texture
		glGenTextures(1, &cubemapID);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapID);

		for (int i = 0; i < 6; ++i)
		{
			auto data = std::vector<unsigned char>();
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB32F, resolution, resolution, 0, GL_RGB, GL_FLOAT, NULL);
		}

		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

		MnemosyEngine::GetInstance().GetIblRenderer().RenderEquirectangularToCubemapTexture(cubemapID, m_equirectangularTextureID, resolution, makeConvolutedIrradianceMap);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapID);
		glGenerateMipmap(GL_TEXTURE_CUBE_MAP);



		if (!m_exportCubemaps)
			return;

		if (makeConvolutedIrradianceMap)
		{
			KtxImage ktxImg;
			ktxImg.SaveCubemapKtx("../Resources/Textures/Cubemaps/spruitSunrise_cube_irradiance.ktx2", cubemapID, resolution);
		}
		else 
		{
			KtxImage ktxImg;
			ktxImg.SaveCubemapKtx("../Resources/Textures/Cubemaps/spruitSunrise_cube_color.ktx2", cubemapID, resolution);
		}

	}

	void Cubemap::equirectangularToPrefilteredCubemap(unsigned int& cubemapID, unsigned int resolution)
	{

		// create cubemap Texture
		glGenTextures(1, &m_prefilterMapID);
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_prefilterMapID);

		for (int i = 0; i < 6; ++i)
		{
			auto data = std::vector<unsigned char>();
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB32F, resolution, resolution, 0, GL_RGB, GL_FLOAT, nullptr);
		}

		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

		glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

		MnemosyEngine::GetInstance().GetIblRenderer().RenderEquirectangularToPrefilteredCubemapTexture(cubemapID, m_equirectangularTextureID, resolution);


		if (!m_exportCubemaps)
			return;


		KtxImage ktxImg;
		ktxImg.SaveCubemapKtx("../Resources/Textures/Cubemaps/spruitSunrise_cube_prefilter.ktx2", cubemapID, resolution);

	}

}