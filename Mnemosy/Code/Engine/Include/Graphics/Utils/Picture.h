#ifndef PICTURE_H
#define PICTURE_H

#include <string>
#include "Include/Graphics/TextureDefinitions.h"

namespace mnemosy::graphics
{
	struct PictureInfo
	{
		uint16_t width = 0;
		uint16_t height = 0;
		TextureFormat textureFormat = TextureFormat::MNSY_NONE;
		void* pixels = nullptr;
	};

	enum PictureErrorCode {
		MNSY_PICTURE_ERROR_NONE = 0,
		MNSY_PICTURE_ERROR_INVALID_TEXTURE_FORMAT,
		MNSY_PICTURE_ERROR_TIFF_WRITE_SCANLINE_FAILED,
		MNSY_PICTURE_ERROR_TIFF_READ_STRIP_FAILED,
		MNSY_PICTURE_ERROR_IMAGESIZE_TOO_BIG,
		MNSY_PICTURE_ERROR_FAILED_TO_OPEN_FILE,
		MNSY_PICTURE_ERROR_FILE_HAS_INVALID_DATA
	};


	class Picture {

	public:
		static std::string ErrorStringGet(const PictureErrorCode picturaErrorCode);


		// Write image data to a tiff file.
		static void WriteTiff(PictureErrorCode& outErrorCode, const char* filepath,const PictureInfo& pictureInfo, const bool flipVertically);

		static PictureInfo ReadTiff(PictureErrorCode& outErrorCode, const char* filepath, const bool flipVertically);


	};


} // ! namespace mnemosy::graphics


#endif //! PICTURE_H