#include "Include/Graphics/Cubemap.h"
#include "Include/Core/Log.h"

#include "Include/MnemosyEngine.h"
#include "Include/Core/FileDirectories.h"
#include "Include/Systems/SkyboxAssetRegistry.h"
#include "Include/Graphics/ImageBasedLightingRenderer.h"
//#include "Include/Graphics/Image.h"
#include "Include/Graphics/Utils/KtxImage.h"

#include "Include/Graphics/Utils/Picture.h"

#include <glad/glad.h>
#include <memory>
#include <filesystem>

namespace mnemosy::graphics
{
	Cubemap::Cubemap() { }

	Cubemap::~Cubemap() {
		if(m_equirectangularTexture_isGenerated)
			glDeleteTextures(1,&m_equirectangularTextureID);
		if(m_colorCubemap_isGenerated)
			glDeleteTextures(1,&m_colorCubemapID);
		if(m_irradianceMap_isGenerated)
			glDeleteTextures(1,&m_irradianceMapID);
		if(m_prefilterMap_isGenerated)
			glDeleteTextures(1,&m_prefilterMapID);

	}

	bool Cubemap::LoadEquirectangularFromFile(const char* imagePath, const char* name,const unsigned int colorCubemapResolution,const bool savePermanently) {
		
		if (m_equirectangularTexture_isGenerated) {
			glDeleteTextures(1, &m_equirectangularTextureID);
			m_equirectangularTextureID = 0;
			m_equirectangularTexture_isGenerated = false;
		}
		
		bool loadingSuccessfull = false;
		{
			graphics::PictureError errorCheck;
						
			graphics::PictureInfo info = graphics::Picture::ReadPicture(errorCheck, imagePath, false,true, false);

			if (!errorCheck.wasSuccessfull) {

				MNEMOSY_ERROR("Failed to load image from file: {} Message: {}", imagePath, errorCheck.what);
				loadingSuccessfull = false;
				return loadingSuccessfull;
			}

			glGenTextures(1, &m_equirectangularTextureID);
			glBindTexture(GL_TEXTURE_2D, m_equirectangularTextureID);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

				
			graphics::TextureFormat format = info.textureFormat;

			uint16_t width = info.width;
			uint16_t height = info.height;

			size_t halfFloatType = GL_UNSIGNED_SHORT;
			if (info.isHalfFloat) {
				halfFloatType = GL_HALF_FLOAT;
			}

			bool invalidFormat = false;
			switch (format)
			{
			case graphics::MNSY_NONE:	invalidFormat = true;	break;
				// 8 bit
			case graphics::MNSY_R8:		invalidFormat = true;	break;
			case graphics::MNSY_RG8:	invalidFormat = true;	break;
			case graphics::MNSY_RGB8:	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8	, width, height, 0, GL_RGB , GL_UNSIGNED_BYTE, info.pixels);	break;
			case graphics::MNSY_RGBA8:	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8	, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, info.pixels);	break;
				// 16 bit
			case graphics::MNSY_R16:	invalidFormat = true;	break;
			case graphics::MNSY_RG16:	invalidFormat = true;	break;
			case graphics::MNSY_RGB16:	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16	, width, height, 0, GL_RGB , halfFloatType, info.pixels);	break;
			case graphics::MNSY_RGBA16:	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16, width, height, 0, GL_RGBA, halfFloatType, info.pixels);	break;
				// 32 bit
			case graphics::MNSY_R32:	invalidFormat = true;	break;
			case graphics::MNSY_RG32:	invalidFormat = true;	break;
			case graphics::MNSY_RGB32:	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F , width, height, 0, GL_RGB , GL_FLOAT, info.pixels);	break;
			case graphics::MNSY_RGBA32:	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, info.pixels);	break;

			default:break;
			}

			free(info.pixels);
				
			if (invalidFormat) {
				MNEMOSY_ERROR("Failed to load image from file: {} - the image has an invalid format for use as a skybox, Format: {}", imagePath, graphics::TexUtil::get_string_from_textureFormat(format));
				loadingSuccessfull = false;
				glDeleteTextures(1, &m_equirectangularTextureID);
				return false;
			}

			glGenerateMipmap(GL_TEXTURE_2D);
			m_equirectangularTexture_isGenerated = true;

		}
		
		// Generate Convolution maps
		ImageBasedLightingRenderer& ibl_Renderer = MnemosyEngine::GetInstance().GetIblRenderer();
		ibl_Renderer.PrepareCubemapRendering();

		equirectangularToCubemap(colorCubemapResolution);
		equirectangularToIrradianceCubemap(m_irradianceMapResolution);
		equirectangularToPrefilteredCubemap(m_prefilteredMapResolution);
		
		ibl_Renderer.EndCubemapRendering();

		if (savePermanently) {
			exportGeneratedCubemapsToKtx( name, colorCubemapResolution);
			MNEMOSY_INFO("Generated and saved skybox {} ", name);
		}

		glDeleteTextures(1, &m_equirectangularTextureID); // we dont need to keep it in gpu memory
		m_equirectangularTextureID = -1;
		m_equirectangularTexture_isGenerated = false;


		loadingSuccessfull = true;
		return loadingSuccessfull;
	}

	void Cubemap::LoadCubemapsFromKtxFiles(const char* colorCubemapPath, const char* irradianceCubemapPath, const char* prefilterCubemapPath)
	{

		// color Cubemap
		{
			if (m_colorCubemap_isGenerated)
			{
				glDeleteTextures(1, &m_colorCubemapID);
				m_colorCubemap_isGenerated = false;
			}
			glGenTextures(1, &m_colorCubemapID);
			//glBindTexture(GL_TEXTURE_CUBE_MAP, m_colorCubemapID); // bound in LoadKtx

			KtxImage ktxImg;
			//ktxImg.LoadKtx(colorCubemapPath, m_colorCubemapID);
			ktxImg.LoadCubemapKTX(colorCubemapPath, m_colorCubemapID);
			//glBindTexture(GL_TEXTURE_CUBE_MAP, m_colorCubemapID);
			//glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			//glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			//glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			//glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			//glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
			
			m_colorCubemap_isGenerated = true;
		}
		//MNEMOSY_TRACE("Loaded color cube")

		// irradiance cubemap
		{
			if (m_irradianceMap_isGenerated)
			{
				glDeleteTextures(1, &m_irradianceMapID);
				m_irradianceMap_isGenerated = false;
			}

			glGenTextures(1, &m_irradianceMapID);
			// glBindTexture(GL_TEXTURE_CUBE_MAP, m_irradianceMapID); // bound in loadKtx

			KtxImage ktxImg;
			ktxImg.LoadCubemapKTX(irradianceCubemapPath, m_irradianceMapID);

			//glBindTexture(GL_TEXTURE_CUBE_MAP, m_irradianceMapID);
			//glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			//glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			//glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			//glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			//glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
			m_irradianceMap_isGenerated = true;
		}
		//MNEMOSY_TRACE("Loaded irradiance cube")

		// Prefilter cubemap
		{
			glGenTextures(1, &m_prefilterMapID);
			//glBindTexture(GL_TEXTURE_CUBE_MAP, m_prefilterMapID); // bound in loadKtx
			
			KtxImage ktxImg;
			ktxImg.LoadKtx(prefilterCubemapPath, m_prefilterMapID);

			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

			m_prefilterMap_isGenerated = true;
		}
		//MNEMOSY_TRACE("Loaded prefilter cube")

	}

	void Cubemap::BindColorCubemap(const unsigned int location)
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
	void Cubemap::BindIrradianceCubemap(const unsigned int location)
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
	void Cubemap::BindPrefilteredCubemap(const unsigned int location)
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

	void Cubemap::equirectangularToCubemap(uint16_t resolution) {
		
		if (m_colorCubemap_isGenerated) {
			glDeleteTextures(1, &m_colorCubemapID);
			m_colorCubemap_isGenerated = false;
			m_colorCubemapID = 0;
		}
		
		// create cubemap texture
		glGenTextures(1, &m_colorCubemapID);
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_colorCubemapID);

		for (int i = 0; i < 6; ++i) {
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB32F, resolution, resolution, 0, GL_RGB, GL_FLOAT, NULL); // preallocate gpu memory
		}

		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

		
		MnemosyEngine::GetInstance().GetIblRenderer().RenderEquirectangularToCubemapTexture(m_colorCubemapID, m_equirectangularTextureID, resolution);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_colorCubemapID);
		glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

		m_colorCubemap_isGenerated = true;
	}

	void Cubemap::equirectangularToIrradianceCubemap(uint16_t resolution) {
		
		if (m_irradianceMap_isGenerated) {
			glDeleteTextures(1, &m_irradianceMapID);
			m_irradianceMap_isGenerated = false;
			m_irradianceMapID = 0;
		}
		// create cubemap texture
		glGenTextures(1, &m_irradianceMapID);
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_irradianceMapID);

		for (int i = 0; i < 6; ++i)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB32F, resolution, resolution, 0, GL_RGB, GL_FLOAT, NULL);
		}

		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

		MnemosyEngine::GetInstance().GetIblRenderer().RenderEquirectangularToIrradianceCubemapTexture(m_irradianceMapID, m_equirectangularTextureID, resolution);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_irradianceMapID);
		glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

		m_irradianceMap_isGenerated = true;

	}

	void Cubemap::equirectangularToPrefilteredCubemap(uint16_t resolution) {

		if (m_prefilterMap_isGenerated) {
			glDeleteTextures(1, &m_prefilterMapID);
			m_prefilterMap_isGenerated = false;
			m_prefilterMapID = 0;
		}
		// create cubemap Texture
		glGenTextures(1, &m_prefilterMapID);
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_prefilterMapID);

		for (int i = 0; i < 6; ++i) {
			auto data = std::vector<unsigned char>();
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB32F, resolution, resolution, 0, GL_RGB, GL_FLOAT, nullptr);
		}

		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

		MnemosyEngine::GetInstance().GetIblRenderer().RenderEquirectangularToPrefilteredCubemapTexture(m_prefilterMapID, m_equirectangularTextureID, resolution);

		m_prefilterMap_isGenerated = true;

	}

	void Cubemap::exportGeneratedCubemapsToKtx(const std::string& name,const unsigned int colorCubemapResolution)
	{

		MNEMOSY_ASSERT(m_colorCubemap_isGenerated,	"Color Cubemap is not yet generated");
		MNEMOSY_ASSERT(m_irradianceMap_isGenerated, "Irradiance Cubemap is not yet generated");
		MNEMOSY_ASSERT(m_prefilterMap_isGenerated,	"Prefilter Cubemap is not yet generated");

		//if (!m_exportCubemaps)
		//	return;
		std::filesystem::path cubemapsPath = MnemosyEngine::GetInstance().GetFileDirectories().GetCubemapsPath();


		
		std::string colorCubemapFileName		= name + "_cubeColor.ktx2";
		std::string irradianceCubemapFileName	= name + "_cubeIrradiance.ktx2";
		std::string prefilterCubemapFileName	= name + "_cubePrefilter.ktx2";
		//MNEMOSY_DEBUG("Color Cubemap File Name: {} ", colorCubemapFileName);

		std::string colorCubemapExportPath		= cubemapsPath.generic_string() + "/" + colorCubemapFileName;
		std::string irradianceCubemapExportPath = cubemapsPath.generic_string() + "/" + irradianceCubemapFileName;
		std::string prefilterCubemapExportPath	= cubemapsPath.generic_string() + "/" + prefilterCubemapFileName;
		//MNEMOSY_DEBUG("Color Cubemap Export Path: {} ", colorCubemapExportPath);
		

		// ToDo Save path to data json

		//MNEMOSY_DEBUG("Saving Color Cubemap With Resolution: {} to: {} ",colorCubemapResolution, colorCubemapExportPath);
		KtxImage colorCubemapKtx;
		colorCubemapKtx.SaveCubemapKtx(colorCubemapExportPath.c_str(), m_colorCubemapID, colorCubemapResolution);

		KtxImage irradianceCubemapKtx;
		irradianceCubemapKtx.SaveCubemapKtx(irradianceCubemapExportPath.c_str(), m_irradianceMapID, m_irradianceMapResolution);

		KtxImage prefilterCubemapKtx;
		prefilterCubemapKtx.SaveCubemapKtx(prefilterCubemapExportPath.c_str(), m_prefilterMapID, m_prefilteredMapResolution);


		MnemosyEngine::GetInstance().GetSkyboxAssetRegistry().AddEntry(name);
	}

}