#include "Include/Graphics/Cubemap.h"
#include "Include/Core/Log.h"

#include "Include/MnemosyEngine.h"
#include "Include/Core/FileDirectories.h"
#include "Include/Systems/SkyboxAssetRegistry.h"
#include "Include/Graphics/Texture.h"
#include "Include/Graphics/ImageBasedLightingRenderer.h"
//#include "Include/Graphics/Image.h"
#include "Include/Graphics/Utils/KtxImage.h"

#include "Include/Graphics/Utils/Picture.h"

#include <glad/glad.h>
#include <memory>
#include <filesystem>

namespace mnemosy::graphics
{
	Cubemap::~Cubemap() {

		if (m_isInitialized) {
			glDeleteTextures(1, &m_gl_ID);
		}
	}

	void Cubemap::GenerateOpenGlCubemap_FromEquirecangularTexture(Texture& equirectangularTex, CubemapType cubeType, bool overrideTexResolution,unsigned int _resolution) {


		MNEMOSY_ASSERT(equirectangularTex.IsInitialized(), "Texture must be initialized first");

		if (m_isInitialized) {
			Clear();
		}

		// set resolution of the cubemap type
		unsigned int resolution = 1024;

		if (cubeType == CubemapType::MNSY_CUBEMAP_TYPE_COLOR) {

			// we are assuming her that a standard equirectangular texture provided will aspect ration of 
			// 2:1 where width has double the pixels of height,
			// therefore we will use height as skybox resolution because higher values will not contain more information than that.

			if (overrideTexResolution && _resolution > 0) {
				resolution = _resolution;
			}
			else {
				resolution = equirectangularTex.GetHeight();
			}

		}
		else if (cubeType == CubemapType::MNSY_CUBEMAP_TYPE_IRRADIANCE) {
			resolution = 64;
		}
		else if (cubeType == CubemapType::MNSY_CUBEMAP_TYPE_PREFILTER) {
			resolution = 512;
		}
		m_resolution = resolution;

		// create cubemap texture
		glGenTextures(1, &m_gl_ID);
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_gl_ID);
		
		
		equirectangularTex.IsHalfFloat();

		uint32_t glInternalFormat = graphics::TexUtil::get_glInternalFormat_from_textureFormat(equirectangularTex.GetTextureFormat());
		uint32_t glFormat = graphics::TexUtil::get_glFormat_from_textureFormat(equirectangularTex.GetTextureFormat());
		uint32_t gldataType = graphics::TexUtil::get_glDataType_from_textureFormat(equirectangularTex.GetTextureFormat(),equirectangularTex.IsHalfFloat());

		// make gl allocate memory for all 6 cube faces
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + 0, 0, glInternalFormat, resolution, resolution, 0, glFormat, gldataType, NULL);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + 1, 0, glInternalFormat, resolution, resolution, 0, glFormat, gldataType, NULL);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + 2, 0, glInternalFormat, resolution, resolution, 0, glFormat, gldataType, NULL);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + 3, 0, glInternalFormat, resolution, resolution, 0, glFormat, gldataType, NULL);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + 4, 0, glInternalFormat, resolution, resolution, 0, glFormat, gldataType, NULL);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + 5, 0, glInternalFormat, resolution, resolution, 0, glFormat, gldataType, NULL);
		//glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + 5, 0, GL_RGB32F, resolution, resolution, 0, GL_RGB, GL_FLOAT, NULL);

		// set filter method
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);


		// Generate Convolution maps
		ImageBasedLightingRenderer& ibl_Renderer = MnemosyEngine::GetInstance().GetIblRenderer();
		
		if (cubeType == CubemapType::MNSY_CUBEMAP_TYPE_COLOR) {

			MnemosyEngine::GetInstance().GetIblRenderer().RenderEquirectangularToCubemapTexture(m_gl_ID, equirectangularTex.GetID(), resolution,true);
		}
		else if (cubeType == CubemapType::MNSY_CUBEMAP_TYPE_IRRADIANCE) {
			
			MnemosyEngine::GetInstance().GetIblRenderer().RenderEquirectangularToIrradianceCubemapTexture( m_gl_ID, equirectangularTex.GetID(), resolution,true);		
		}
		else if (cubeType == CubemapType::MNSY_CUBEMAP_TYPE_PREFILTER) {

			
			MnemosyEngine::GetInstance().GetIblRenderer().RenderEquirectangularToPrefilteredCubemapTexture(m_gl_ID, equirectangularTex.GetID(), resolution);

		}

		m_isInitialized = true;
	}



	void Cubemap::GenerateOpenGlCubemap_FromKtx2File(std::filesystem::path& path, bool loadStoredMips)
	{
		if (!std::filesystem::exists(path)) {
			MNEMOSY_WARN("Cubemap could not load because filepath does not exist. Path: {} ", path.generic_string());
			return;
		}

		Clear();

		glGenTextures(1, &m_gl_ID);

		bool genMipMaps = !loadStoredMips;

		
		KtxImage ktxImg;
		bool success =  ktxImg.LoadCubemap(path.generic_string().c_str(), m_gl_ID,loadStoredMips, genMipMaps);
		
		m_resolution = ktxImg.width;

		if (!success) {
			MNEMOSY_ERROR("Cubemap:LoadFromKtx2File: faild to load. Path: {} ", path.generic_string());
			Clear();
		}

		m_isInitialized = true;
	}

	void Cubemap::Clear()
	{
		if (m_isInitialized) {

			if (m_gl_ID != 0) {
				Unbind();
				glDeleteTextures(1, &m_gl_ID);
				m_gl_ID = 0;
			}
			m_lastBoundLocation = 0;
			m_resolution = 0;
			m_isInitialized = false;
		}
	}

	void Cubemap::Bind(const unsigned int location)
	{
		MNEMOSY_ASSERT(m_isInitialized, "Cubemap must be initialized to bind it.");

		if (location > 16 || location < 0) {

			MNEMOSY_ERROR("Cubemap::Bind: Cannot bind to location {}, only range 0 - 16 are allowed ");
			return;
		}

		glActiveTexture(GL_TEXTURE0 + location);
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_gl_ID);

		m_lastBoundLocation = (uint8_t)location;
	}

	void Cubemap::Unbind()
	{
		MNEMOSY_ASSERT(m_isInitialized, "Cubemap must be initialized to unbind it.");

		glActiveTexture(GL_TEXTURE0 + m_lastBoundLocation);
		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	}
}