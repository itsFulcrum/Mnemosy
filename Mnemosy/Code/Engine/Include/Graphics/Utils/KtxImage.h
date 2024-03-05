#ifndef KTX_IMAGE_H
#define KTX_IMAGE_H

namespace mnemosy::graphics
{
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

		unsigned int numChannels = 0;
		unsigned int width = 0;
		unsigned int height = 0;
	};

} // mnemosy::graphics

#endif // !KTX_IMAGE_H
