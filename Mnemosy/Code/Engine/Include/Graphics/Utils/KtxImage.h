#ifndef KTX_IMAGE_H
#define KTX_IMAGE_H

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
		const bool LoadCubemapKTX(const char* filepath, unsigned int& glTextureID);
		const bool SaveKtx(const char* filepath, unsigned char* imageData, unsigned int numChannels, unsigned int width, unsigned int height);
		const bool SaveBrdfLutKtx(const char* filepath, unsigned int& glTextureID, unsigned int resolution);
		const bool SaveCubemapKtx(const char* filepath, unsigned int& glTextureID, unsigned int resolution);

		const bool ExportGlTexture(const char* filepath, unsigned int glTextureID, const unsigned int numChannels,const unsigned int width,const unsigned int height, ktxImgFormat imgFormat, bool exportMips);


		unsigned int numChannels = 0;
		unsigned int width = 0;
		unsigned int height = 0;
	};

} // mnemosy::graphics

#endif // !KTX_IMAGE_H
