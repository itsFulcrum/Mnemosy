#ifndef KTX_IMAGE_H
#define KTX_IMAGE_H

#include <stdint.h>


namespace mnemosy::graphics {
	enum TextureFormat;
	enum VkFormat;
}

namespace mnemosy::graphics
{
	enum ktxImgFormat {
		MNSY_COLOR,			// RGB8 or RGBA8 - for color textures 
		MNSY_COLOR_SRGB,	// RGB8 or RGBA8 - for color textures, but srgb encoded
		MNSY_NORMAL,		// RGB32 - for normal maps
		MNSY_LINEAR_CHANNEL // R32 - for roghtness, metallic etc
	};

	class KtxImage
	{
	public:
		KtxImage() = default;
		~KtxImage() = default;

		const bool LoadKtx(const char* filepath, unsigned int &glTextureID);
		const bool LoadBrdfKTX(const char* filepath, unsigned int& glTextureID);
		//const bool LoadCubemapKTX(const char* filepath, unsigned int& glTextureID);
		
		// Set loadStoredMips to true to load mipmaps from file, if we want to generate them set loadStoredMips false and  genMips true.
		// this method already uploads to gpu using glTextureID.
		const bool LoadCubemap(const char* filepath, unsigned int& glTextureID, bool loadStoredMips, bool genMips);


		const bool SaveKtx(const char* filepath, unsigned char* imageData, unsigned int numChannels, unsigned int width, unsigned int height);
		const bool SaveBrdfLutKtx(const char* filepath, unsigned int& glTextureID, unsigned int resolution);
		const bool SaveCubemap(const char* filepath, unsigned int& glTextureID, TextureFormat format, unsigned int resolution, bool storeMipMaps);

		const bool ExportGlTexture(const char* filepath, unsigned int glTextureID, const unsigned int numChannels,const unsigned int width,const unsigned int height, ktxImgFormat imgFormat, bool exportMips);


		// general interface implementation for use of picture library,  Support no mipmaps but in theory all possible mnemosy TextureFormat.
		// return ktx_error_code_e (enum)
		unsigned int Save_WithoutMips(const char* filepath, void* pixels, const bool flipVertically, const TextureFormat format, const uint16_t _width, const uint16_t _height, const bool isHalfFloat);


		TextureFormat GetMnemosyFormatFromVkFormat(VkFormat vkFormat);
		VkFormat GetVkFormatFromMnemosyFormat(TextureFormat mnsyFormat, bool isHalfFloat);

		uint16_t width	= 0;
		uint16_t height = 0;
		uint8_t numChannels = 0;
	};

} // mnemosy::graphics

#endif // !KTX_IMAGE_H
