#include "Include/Graphics/Utils/Picture.h"

#include "Include/Core/Log.h"
#include "tiffio.h"


namespace mnemosy::graphics {


	// not using this library for now but my be using it in the future if I can figure out how to properly read tiff files with libTiff manually
	std::string Picture::ErrorStringGet(const PictureErrorCode picturaErrorCode)
	{
		switch (picturaErrorCode)
		{
		case MNSY_PICTURE_ERROR_NONE: return std::string("Success");
		case MNSY_PICTURE_ERROR_INVALID_TEXTURE_FORMAT: return std::string("Texture format is invalid or not supported");
		case MNSY_PICTURE_ERROR_TIFF_WRITE_SCANLINE_FAILED: return std::string("An Error occured while writing a tiff scanline");
		case MNSY_PICTURE_ERROR_TIFF_READ_STRIP_FAILED: return std::string("An Error occured while reading a tiff endcoded strip");
		case MNSY_PICTURE_ERROR_IMAGESIZE_TOO_BIG: return std::string("Image size is too big");
		case MNSY_PICTURE_ERROR_FAILED_TO_OPEN_FILE: return std::string("Failed to open file");
		case MNSY_PICTURE_ERROR_FILE_HAS_INVALID_DATA: return std::string("File contains invalid data");
		}

		return std::string("None");
	}

	void Picture::WriteTiff(PictureErrorCode& outErrorCode, const char* filepath, const PictureInfo& pictureInfo, const bool flipVertically)
	{
		outErrorCode = MNSY_PICTURE_ERROR_NONE;

		TextureFormat format = pictureInfo.textureFormat;

		if (format == TextureFormat::MNSY_NONE) {
			outErrorCode = MNSY_PICTURE_ERROR_INVALID_TEXTURE_FORMAT;
			return;
		}

		// amount of color channels
		uint8_t channels = (uint8_t)format % 4;
		if (channels == 0) {
			channels = 4;
		}

		// asume 8 bits per pixel
		uint8_t bitsPerChannel = 8;
		size_t bytesPerPixel = channels * sizeof(uint8_t);

		if(format == MNSY_R16 || format == MNSY_RG16 || format ==  MNSY_RGB16 || format == MNSY_RGBA16){
			bitsPerChannel = 16;
			bytesPerPixel = channels * sizeof(uint16_t);
		}
		else if(format == MNSY_R32 || format == MNSY_RG32 || format == MNSY_RGB32 || format == MNSY_RGBA32){
			bitsPerChannel = 32;
			bytesPerPixel = channels * sizeof(uint32_t);
		}


		// this should make sure we are allowed to allocate large amounts of data
		tmsize_t limit = (bytesPerPixel * pictureInfo.width * pictureInfo.height);
		TIFFOpenOptions* opts = TIFFOpenOptionsAlloc();
		TIFFOpenOptionsSetMaxSingleMemAlloc(opts, limit);


		TIFF* tif = TIFFOpenExt(filepath, "w",opts);
		TIFFOpenOptionsFree(opts);


		TIFFSetField(tif, TIFFTAG_IMAGEWIDTH, pictureInfo.width);
		TIFFSetField(tif, TIFFTAG_IMAGELENGTH, pictureInfo.height);
		TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, channels); // channels per pixel (1,2,3,4)
		TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, bitsPerChannel); // bits per channel (8 , 16 or 32)

		TIFFSetField(tif, TIFFTAG_ROWSPERSTRIP, 1);
		TIFFSetField(tif, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);
		TIFFSetField(tif, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);

		// for grayscale images set different tag
		if (format == graphics::MNSY_R8 || format == graphics::MNSY_R16 || format == graphics::MNSY_R32) {
			TIFFSetField(tif, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK);
		}
		else {
			TIFFSetField(tif, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);
		}

		TIFFSetField(tif, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_UINT);
		TIFFSetField(tif, TIFFTAG_COMPRESSION, COMPRESSION_NONE);

		// buffer with the size of one row of pixels
		size_t bytesPerRow = pictureInfo.width * bytesPerPixel;
		void* rowBuffer = malloc(bytesPerRow);
		
		// now we coppy row by row the data from the main pixel buffer to this row buffer and hand that rowbuffer to the tiff library

		int error = 0;
		//			not Tested			  Not Tested			   works			     works
		if (format == MNSY_R8 || format == MNSY_RG8 || format == MNSY_RGB8 || format == MNSY_RGBA8) {
			
			size_t offset = 0;
			for (uint16_t i = 0; i < pictureInfo.height; i++) {

				// awsume we want to flip vertically - normally i do.
				// also we dont have to multiply by bytes per channel because its already part of memcopy where we cast the pointer to a type of (uint8_t) in this case
				offset = (pictureInfo.height - i - 1) * channels * pictureInfo.width;
				if (!flipVertically) {
					offset = i * channels * pictureInfo.width;
				}
								
				memcpy(rowBuffer, (uint8_t*)pictureInfo.pixels + offset, bytesPerRow);
				error = TIFFWriteScanline(tif, rowBuffer, i, 0);
			}
		}
		//				works						not tested			works					not tested
		else if (format == MNSY_R16 || format == MNSY_RG16 || format == MNSY_RGB16 || format == MNSY_RGBA16) {
			
			size_t offset = 0;
			for (uint16_t  i = 0; i < pictureInfo.height; i++) {

				// awsume we want to flip vertically - normally i do.
				// also we dont have to multiply by bytes per channel because its already part of memcpy where we cast the pointer to a type of (uint16_t) in this case
				offset = (pictureInfo.height - i - 1) * channels * pictureInfo.width;
				if (!flipVertically) {
					offset = i * channels * pictureInfo.width;
				}

				memcpy(rowBuffer, (uint16_t*)pictureInfo.pixels + offset, bytesPerRow);
				error = TIFFWriteScanline(tif, rowBuffer, i, 0);
			}
		}

		// not tested at all
		else if (format == MNSY_R32 || format == MNSY_RG32 || format == MNSY_RGB32 || format == MNSY_RGBA32) { 

			size_t offset = 0;
			for (uint16_t i = 0; i < pictureInfo.height; i++) {

				// awsume we want to flip vertically - normally i do.
				// also we dont have to multiply by bytes per channel because its already part of memcpy where we cast the pointer to a type of (uint32_t) in this case
				offset = (pictureInfo.height - i - 1) * channels * pictureInfo.width;
				if (!flipVertically) {
					offset = i * channels * pictureInfo.width;
				}

				memcpy(rowBuffer, (uint32_t*)pictureInfo.pixels + offset, bytesPerRow);
				error = TIFFWriteScanline(tif, rowBuffer, i, 0);
			}
		}

		TIFFClose(tif); // write the file

		if (error == -1) {
			outErrorCode = MNSY_PICTURE_ERROR_TIFF_WRITE_SCANLINE_FAILED;
		}

		free(rowBuffer); 
	}

	PictureInfo Picture::ReadTiff(PictureErrorCode& outErrorCode, const char* filepath, const bool flipVertically)
	{

		outErrorCode = PictureErrorCode::MNSY_PICTURE_ERROR_NONE;
		
		PictureInfo info;
		info.textureFormat = graphics::TextureFormat::MNSY_NONE;
		info.width = 0;
		info.height = 0;
		info.pixels = nullptr;

		tmsize_t limit = (sizeof(uint32_t) * 4 * 16384 * 16384); // sizeof(uint32_t) * channels * width * height

		TIFFOpenOptions* opts = TIFFOpenOptionsAlloc();
		TIFFOpenOptionsSetMaxSingleMemAlloc(opts, limit);

		TIFF* tif = TIFFOpenExt(filepath, "r", opts);
		TIFFOpenOptionsFree(opts);


		if (tif == nullptr) {
			outErrorCode = PictureErrorCode::MNSY_PICTURE_ERROR_FAILED_TO_OPEN_FILE;
			return info;
		}


		// Using the RGBA interface works nicely but it always outputs RGBA 8 bit   no 16 bit is supported so it is kindof a dealbreaker
		// I should just leave this alone and keep using some higer level library like openCV for now
		// maybe look at openImageIo which is what blender also uses

		TIFFRGBAImage img;
		char emsg[1024];

		if (TIFFRGBAImageBegin(&img, tif, 0, emsg)) {
			size_t npixels;
			uint32_t* raster;
			npixels = img.width * img.height;

			raster = (uint32_t*)_TIFFmalloc(npixels * sizeof(uint32_t));
			if (raster != NULL) {
				if (TIFFRGBAImageGet(&img, raster, img.width, img.height)) {
						
					// put it into my pixelbuffer

					info.textureFormat = MNSY_RGBA8;

					info.width = img.width;
					info.height = img.height;


					//void* pixelBuffer = (uint32_t*)_TIFFmalloc(npixels * sizeof(uint32_t));
					//_TIFFmemcpy(pixelBuffer, raster, npixels * sizeof(uint32_t));
						
					info.pixels = raster;

				}
				//_TIFFfree(raster);
			}
			TIFFRGBAImageEnd(&img);
		}
		else {
			//TIFFError(argv[1], emsg);
		}
		TIFFClose(tif);

		return info;
		
		/*
		// this should make sure we are allowed to allocate large amounts 

		tmsize_t limit = (sizeof(uint32_t) * 4 * 16384 * 16384); // sizeof(uint32_t) * channels * width * height

		TIFFOpenOptions* opts = TIFFOpenOptionsAlloc();
		TIFFOpenOptionsSetMaxSingleMemAlloc(opts, limit);

		TIFF* tif = TIFFOpenExt(filepath, "r", opts);
		TIFFOpenOptionsFree(opts);

		if (tif == nullptr) {
			outErrorCode = PictureErrorCode::MNSY_PICTURE_ERROR_FAILED_TO_OPEN_FILE;
			return info;
		}
		
		uint32_t width = 0;
		uint32_t height = 0;
		TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &width);
		TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &height);

		if (width == 0 || height == 0) {
			outErrorCode = PictureErrorCode::MNSY_PICTURE_ERROR_FILE_HAS_INVALID_DATA;
			TIFFClose(tif);
			return info;
		}

		uint32_t channels = 0;
		uint32_t bitsPerChannel = 0;
		TIFFGetField(tif, TIFFTAG_SAMPLESPERPIXEL, &channels); // channels per pixel (1,2,3,4)
		TIFFGetField(tif, TIFFTAG_BITSPERSAMPLE, &bitsPerChannel); // bits per channel (8 , 16 or 32)

		// determine the texture format
		graphics::TextureFormat format = graphics::TextureFormat::MNSY_NONE;
		{
			if (channels == 1) {
				switch (bitsPerChannel)
				{
				case 8:  format = graphics::MNSY_R8; break;
				case 16: format = graphics::MNSY_R16; break;
				case 32: format = graphics::MNSY_R32; break;
				}
			}
			else if (channels == 2) {
				switch (bitsPerChannel)
				{
				case 8:  format = graphics::MNSY_RG8; break;
				case 16: format = graphics::MNSY_RG16; break;
				case 32: format = graphics::MNSY_RG32; break;
				}
			}
			else if (channels == 3) {
				switch (bitsPerChannel)
				{
				case 8:  format = graphics::MNSY_RGB8; break;
				case 16: format = graphics::MNSY_RGB16; break;
				case 32: format = graphics::MNSY_RGB32; break;
				}
			}
			else if (channels == 4) {
				switch (bitsPerChannel)
				{
				case 8:  format = graphics::MNSY_RGBA8; break;
				case 16: format = graphics::MNSY_RGBA16; break;
				case 32: format = graphics::MNSY_RGBA32; break;
				}
			}

			if (format == graphics::TextureFormat::MNSY_NONE) {
				outErrorCode = PictureErrorCode::MNSY_PICTURE_ERROR_FILE_HAS_INVALID_DATA;
				TIFFClose(tif);
				return info;
			}
		}

		// fill info struct
		info.textureFormat = format;
		info.width  = (uint16_t)width;
		info.height = (uint16_t)height;

		*/
		// use RGBA interface


		

		// read the file

		/*
		{

			size_t bufferSize = TIFFStripSize(tif);
			void* buffer = _TIFFmalloc(bufferSize);

			uint32_t numberOfStrips = TIFFNumberOfStrips(tif);
			uint32_t config = 0;
			TIFFGetField(tif, TIFFTAG_PLANARCONFIG, &config);

			tmsize_t t = 0;
			for (uint32_t strip = 0; strip < numberOfStrips; strip++) {

				t =TIFFReadEncodedStrip(tif,strip, buffer, (tsize_t)-1);
			}
		
			info.pixels = buffer;

			if (t == -1) {
				outErrorCode = PictureErrorCode::MNSY_PICTURE_ERROR_TIFF_READ_STRIP_FAILED;
				free(buffer);
				info.pixels = nullptr;
			}

			// now we need flip the image but for now just test if it works
				// memcopy data to a new final pixel buffer

			TIFFClose(tif);
		
			return info;
		}
		*/

		// below may be used but could cause problems when planerconfig is separate AND "if RowsPerStrip was not one because the order in which scanlines are requested would require random access to data within strips (something that is not supported by the library when strips are compressed)"
		// https://libtiff.gitlab.io/libtiff/libtiff.html#tiff-tags
		// also it doesn't seem to work at all
		/*
		{
			void* buffer = nullptr;
			size_t bufferSize = TIFFScanlineSize(tif);
			buffer = _TIFFmalloc(bufferSize);


			uint32_t config = 0;
			TIFFGetField(tif, TIFFTAG_PLANARCONFIG, &config);

			if (config == PLANARCONFIG_CONTIG) { // if stored contiguesly

				uint32_t row = 0;
				for (row = 0; row < height; row++) {
					TIFFReadScanline(tif, buffer, row, 0);
				}
			}
			else if (config == PLANARCONFIG_SEPARATE) {

				uint16_t s = 0;
				uint16_t nsamples = channels;

				for (s = 0; s < nsamples; s++) {

					for (uint32_t row = 0; row < height; row++) {
						TIFFReadScanline(tif, buffer, row, s);
					}
				}
			}
			info.pixels = buffer;

			TIFFClose(tif);
			
			return info;
		}
		*/

	}

}