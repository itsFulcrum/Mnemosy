#include "Include/Graphics/Cubemap.h"
#include "Include/Core/Log.h"

#include "Include/MnemosyEngine.h"
#include "Include/Core/FileDirectories.h"
#include "Include/Systems/SkyboxAssetRegistry.h"
#include "Include/Graphics/ImageBasedLightingRenderer.h"
#include "Include/Graphics/Image.h"
#include "Include/Graphics/Utils/KtxImage.h"

#include <glad/glad.h>
#include <memory>

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
			m_equirectangularTexture_isGenerated = false;
		}
		
		bool loadingSuccessfull = false;
		{
			// TODO : Load using OpenCV not stbImage -> move all image loading to either ktx or OpenCv 
			std::unique_ptr<Image>  img = std::make_unique<Image>();
			bool successfull = img->LoadImageFromFileFLOAT(imagePath, false);
			if (!successfull) {
				MNEMOSY_ERROR("Failed to load image from file: {}", imagePath);
				loadingSuccessfull = false;
				return loadingSuccessfull;
			}

			glGenTextures(1, &m_equirectangularTextureID);
			glBindTexture(GL_TEXTURE_2D, m_equirectangularTextureID);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, img->width, img->height, 0, GL_RGB, GL_FLOAT, img->imageDataFLOAT); // upload data to gpu
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
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

	void Cubemap::equirectangularToCubemap(unsigned int resolution) {
		
		if (m_colorCubemap_isGenerated) {
			glDeleteTextures(1, &m_colorCubemapID);
			m_colorCubemap_isGenerated = false;
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

	void Cubemap::equirectangularToIrradianceCubemap(unsigned int resolution) {
		
		if (m_irradianceMap_isGenerated) {
			glDeleteTextures(1, &m_irradianceMapID);
			m_irradianceMap_isGenerated = false;
		}
		// create cubemap texture
		glGenTextures(1, &m_irradianceMapID);
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_irradianceMapID);

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

		MnemosyEngine::GetInstance().GetIblRenderer().RenderEquirectangularToIrradianceCubemapTexture(m_irradianceMapID, m_equirectangularTextureID, resolution);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_irradianceMapID);
		glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

		m_irradianceMap_isGenerated = true;		

	}

	void Cubemap::equirectangularToPrefilteredCubemap(unsigned int resolution) {

		if (m_prefilterMap_isGenerated) {
			glDeleteTextures(1, &m_prefilterMapID);
			m_prefilterMap_isGenerated = false;
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
		fs::path cubemapsPath = MnemosyEngine::GetInstance().GetFileDirectories().GetCubemapsPath();


		
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