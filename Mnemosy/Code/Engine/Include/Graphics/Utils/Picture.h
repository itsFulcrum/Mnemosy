#ifndef PICTURE_H
#define PICTURE_H

#include "Include/Graphics/TextureDefinitions.h"

#include <string>


// In development
/*
	
	Wrapper for loading images using different image libraries under a common interface

	Currently supported formats: tif, png, jpg, exr, hdr
	ktx2 is not yet supported.
	using STB_Read() other formats like tga and bmp are also possible but only in RGB8 / RGBA8 format.

	Image Libaries currently used are:
	OpenExr
	libTiff
	lodepng
	stb_image.h
	stb_image_write.h

	See bottom of this file for quick usage example.
*/

namespace mnemosy::graphics
{
	struct PictureInfo
	{
	public:
		uint16_t width = 0;
		uint16_t height = 0;
		TextureFormat textureFormat = TextureFormat::MNSY_NONE;
		bool isHalfFloat = false;
		void* pixels = nullptr;
	};

	struct PictureError
	{
	public:
		bool wasSuccessfull = true;
		std::string what = "";
	};

	class Picture {

	public:
		

		// General interface to read image files. 
		// for exr and hdr files data is often returned in linear Color space. by default data is converted to sRGB to turn this off PBRTypeHint must be set to MNSY_TEXTURE_NONE
		static PictureInfo ReadPicture(PictureError& outPictureError, const char* filepath, const bool flipVertically, const graphics::PBRTextureType PBRTypeHint);

		// Same as ReadPicture() but instead of returning PictureInfo struct, it is passed as second parameter so we can use this procedure for multithreading when loading several images simulaniously
		static void ReadPicture_thread(PictureError& outPictureError, PictureInfo& outPicInfo, const std::string filepath, const bool flipVertically, const graphics::PBRTextureType PBRTypeHint);

		// general interface for writing images so it supports multiple formats
		// Formats Exr and Hdr are assumed to be passes values in sRBG color space and will always convert to linear as this is the standard for these formats.
		// for now, if data is already in linear space the respective write methods for exr and hdr must be called directly to specify wheather to convert to linear or not
		static void WritePicture(PictureError& outPictureError, const char* filepath, const PictureInfo& pictureInfo, const bool flipVertically, const graphics::PBRTextureType PBRTypeHint);


		// Read .tif images. Supports full RGBA channels with up to 32 bit.
		static PictureInfo ReadTiff(PictureError& outPictureError, const char* filepath, const bool flipVertically, const bool convertGrayToRGB);

		// Write image data to a tiff file. Using scaline interface
		static void WriteTiff(PictureError& outPictureError, const char* filepath,const PictureInfo& pictureInfo, const bool flipVertically);


		// Reads .exr image files, returns pixel buffer as either half_float (16 bit) or float (32 bit), pictureInfo.isHalfFloat is true for 16bit images
		// The implementation only supports a subset of possilbe .exr configurations.
		// Supported are channels R, RG, RGB, RGBA as either 16 or 32 bit, type UINT is not supported.  
		// Furthemore channels are searched for in the order R,G,B and last A, if the previous channel did not exist all following will not be considered. 
		// (e.g. if the file has channels R B A only R will be returned)
		// G & B channel must have the same bit depth (16/32) as R channel otherwise they will not be counted.
		// Alpha channel will be promoted or demoted if it is not the same as R
		// If no R channel is present we look for gray channels 'Y' and 'Z' if a gray channel is found it is given back as R16 or R32 or if convertGrayToRGB is true as RGB16 or RGB32
		static PictureInfo ReadExr(PictureError& outPictureError, const char* filepath, const bool flipVertically,const bool convertToSrgb,const bool convertGrayToRGB);

		// Write .exr files. Exr should be provided with 32 bit float or 16 bit half-float data. Not 16 bit unsinged short data.
		// CAUTION: feeding WriteExr with 8 bit pixels will work and try to promote to 16 bit but may produce unexpected results especially when combined with convert to linear
		static void WriteExr(PictureError& outPictureError, const char* filepath, const PictureInfo& pictureInfo, const bool flipVertically, const bool convertToLinear);

		// Read .hdr files.  This will always return RGB32 format.
		static PictureInfo ReadHdr(PictureError& outPictureError, const char* filepath, const bool flipVertically, const bool convertToSrgb);
		
		// Write .hdr files. Expects 32 bit floating point data. 8 and 16 bit data will be cause failure.
		// single channels will be duplicated to RGB and alpha channel will be discarded.
		static void WriteHdr(PictureError& outPictureError, const char* filepath, const PictureInfo& pictureInfo, const bool flipVertically, const bool convertToLinear);

		// Read .jpg files.  Always returns R8 or RGB8
		static PictureInfo ReadJpg(PictureError& outPictureError, const char* filepath, const bool flipVertically, const bool convertGrayToRGB);
		
		// Write .jpg files. Expects 8 bit input data but can convert 16 bit to 8 bit. 32 bit data will cause failure. 
		// single channel will be duplicated to RGB and alpha channels will be discarded.
		static void WriteJpg(PictureError& outPictureError, const char* filepath, const PictureInfo& pictureInfo, const bool flipVertically);

		// Read .png files. Supports most Png (Gray,RGB/RGBA as 8 and 16 bit). 1,2,3,4 bit grayscale will be promoted to 8bit as R8.
		// Grayscale + Alpha - alpha channel is discarded.   
		// 16 bit images are converted from big to little endian.
		// all pallet encoded images are converted to 8bit RGB (not tested yet)
		static PictureInfo ReadPng(PictureError& outPictureError, const char* filepath, const bool flipVertically, const bool convertGrayToRGB);
		
		// Write .png files. Supports R (gray) , RGB and RGBA in both 8 and 16 bit.  32 bit formats are rejected. Dual Channel export is also not supported.
		// 16 bit are automatically converted from little to big endian for png images.
		static void WritePng(PictureError& outPictureError, const char* filepath, const PictureInfo& pictureInfo, const bool flipVertically);

		
		// always returns RGB8 or RGBA8
		// 16 bit get demoted to 8 bit. Usefull when we always want 8 bit (for glfw window icon e.g) other Read methods will always return the highest possilbe bit depth
		static PictureInfo STB_Read(PictureError& outPictureError, const char* filepath, const bool flipVertically);

		// ========== Utils ==========

		static PictureError pic_util_check_input_pictureInfo(const PictureInfo& pictureInfo);

		static float pic_util_linear2srgb_float(float linearValue);
		static float pic_util_srgb2linear_float(float srgbValue);

	};


} // ! namespace mnemosy::graphics


#endif //! PICTURE_H


// Usage
/*


	Each format has its own function for read / write.
	ReadTiff()
	WriteTiff()
	ReadExr()
	WriteExr()
	ReadJpg()
	WriteJpg()
	ReadPng()
	WritePng()
	ReadHdr()
	WriteHdr()

	Each method has esentially the same parameters.
	For High dinamic range format .hdr and .exr and additonal bool for converting srgb and linear is provided.

	There are two general functions
	ReadPicture()
	WritePicture() 
	that check the file extention in the provided filepath and delegate to the appropriate method.

	each method also has a quick explaination on the supported format etc.

	
	== reading an image 

	bool flipVertically = true;
	graphics::PBRTextureType texTypeHint = graphics::PBRTextureType::MNSY_TEXTURE_ALBEDO
	this interface was developed for the purpose of loading texture for 3D graphics.
	Using the texType hint we can provide hint how to return the data.
	esentially if its set to MNSY_TEXTURE_ALBEDO or MNSY_TEXTURE_EMISSION  so a texture with Srgb color then when loading gray scale image will be converted from grayscale (R) to RGB by replicating it to the other channels.
	also for hdr and exr data is always converted from linear to srgb unless TexTypeHint is set to MNSY_TEXTURE_NONE;
	because usually we want to use the data in srgb. this is only the case for the General function ReadPicture()  using the dedicated funciton ReadExr() we can specify wheather to convert or not.


	PictureError errorCheck;
	PictureInfo picInfo = ReadPicture(errorCheck, "C:/image.png", flipVertically, texTypeHint)
	if(!errorCheck.wasSuccesful){
		printf("error occured. Message: %s", errorCheck.c_str());
		
		if an error occure picInfo will be empty no pixel date will be in memory anymore. so there is nothing to free
	}

	// not we can access the pixel data in the PicInfo Struct.

	void* pixels = picInfo.pixels;
	int width = picInfo.width;
	int height = picInfo.height;
	bool isHalfFloat = picInfo.isHalfFloat // exr returns half float instead of unsigned short , this may be important depending on the use case for example when uploading to the GPU
	picInfo.textureFormat => this is an enum containing all possilbe formats R8,RG8..RGB16.. to RGBA32
	
	==== Write an image



	




*/