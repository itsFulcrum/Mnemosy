#include "Include/Graphics/Image.h"

#include <tiff.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace mnemosy::graphics
{
	Image::Image()
	{ }
	Image::~Image()
	{
		FreeData();
	}

	bool Image::LoadImageFromFile(const char* imagePath, bool flipImageVertically) {

		stbi_set_flip_vertically_on_load(flipImageVertically);
		
		int width_, height_, channels_;

		imageData = stbi_load(imagePath, &width_, &height_, &channels_, 0);

		if (imageData) {

			channels = unsigned int(channels_);
			width = unsigned int(width_);
			height = unsigned int(height_);

			return true;
		}
		else {

			imageData = nullptr;
			return false;
		}
		return false;
	}

	bool Image::LoadImageFromFileFLOAT(const char* imagePath, bool flipImageVertically)
	{
		stbi_set_flip_vertically_on_load(flipImageVertically);
		int width_, height_, channels_;

		imageDataFLOAT = stbi_loadf(imagePath, &width_, &height_, &channels_, 0);

		if (imageDataFLOAT)
		{
			channels = unsigned int(channels_);
			width = unsigned int(width_);
			height = unsigned int(height_);

			return true;
		}
		else
		{
			imageDataFLOAT = nullptr;
			return false;
		}
		return false;
	}

	void Image::FreeData()
	{

		if (imageData)
		{
			stbi_image_free(imageData);
		}
		if (imageDataFLOAT)
		{
			stbi_image_free(imageDataFLOAT);
		}

		width = 0;
		height = 0;
		channels = 0;
	}

} // !mnemosy::graphics