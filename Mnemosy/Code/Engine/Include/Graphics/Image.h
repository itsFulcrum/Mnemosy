#ifndef GRAPHICS_IMAGE_H
#define GRAPHICS_IMAGE_H

namespace mnemosy::graphics
{
	class Image
	{
	public:
		Image();
		~Image();

		bool LoadImageFromFile(const char* imagePath, bool flipImageVertically);
		bool LoadImageFromFileFLOAT(const char* imagePath, bool flipImageVertically);

		void FreeData();

		unsigned char* imageData = nullptr;
		float* imageDataFLOAT = nullptr;
		unsigned int width = 0;
		unsigned int height = 0;
		unsigned int channels = 0;
	};
} // mnemosy::graphics

#endif // !GRAPHICS_IMAGE_H
