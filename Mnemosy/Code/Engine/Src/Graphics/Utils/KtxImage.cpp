#include "Include/Graphics/Utils/KtxImage.h"
#include "Include/Core/Log.h"

#include "Include/MnemosyEngine.h"
#include "Include/Core/Clock.h"

#include "Include/Graphics/TextureDefinitions.h"

#include <filesystem>
#include <math.h>
#include <stdio.h>


#include <glad/glad.h>

#include <ktx.h>
#include <vkformat_enum.h>
//#include <vk_format.h>
//#include <ktxvulkan.h>
#include <ktxint.h>

#include <half.h>


namespace mnemosy::graphics
{
	const bool KtxImage::LoadKtx(const char* filepath, unsigned int& glTextureID) {
		
		ktxTexture2* kTexture = nullptr;
		KTX_error_code errorCode;
		
		errorCode = ktxTexture2_CreateFromNamedFile(filepath, KTX_TEXTURE_CREATE_ALLOC_STORAGE, &kTexture);
		//errorCode = ktxTexture_CreateFromNamedFile(filepath, KTX_TEXTURE_CREATE_ALLOC_STORAGE, &kTexture);
		if (errorCode != 0) {

			MNEMOSY_ERROR("KtxImage::LoadKtx - CreatFromNamedFile Failed\nPath:{} \nError code: {}",filepath, ktxErrorString(errorCode));
			numChannels = 0; width = 0; height = 0;
			if(kTexture)
				ktxTexture_Destroy(ktxTexture(kTexture));
			return false;
		}

		graphics::TextureFormat mnsyFormat = GetMnemosyFormatFromVkFormat((VkFormat)kTexture->vkFormat);
		if (mnsyFormat == graphics::TextureFormat::MNSY_NONE) {
			MNEMOSY_ERROR("KtxImage::LoadCubemap: Faild. Unsupported texture format.");
			return false;
		}



		
		GLenum target, glerror;
		errorCode = ktxTexture_GLUpload(ktxTexture(kTexture), &glTextureID, &target, &glerror);
		if (errorCode != 0)
		{
			MNEMOSY_ERROR("KtxImage::LoadKtx: GLUpload Failed \nError code: {}", ktxErrorString(errorCode));
			numChannels = 0; width = 0; height = 0;
			ktxTexture_Destroy(ktxTexture(kTexture));
			return false;
		}
		

		uint8_t _numChannels, _bitsPerChannel, _bytesPerPixel;
		graphics::TexUtil::get_information_from_textureFormat(mnsyFormat, _numChannels, _bitsPerChannel, _bytesPerPixel);
		
		width = kTexture->baseWidth;
		height = kTexture->baseHeight;
		numChannels = _numChannels;

		ktxTexture_Destroy(ktxTexture(kTexture));
		return true;
		
	}

	const bool KtxImage::LoadBrdfKTX(const char* filepath, unsigned int& glTextureID)
	{
		// function upload the image directly to openGl instead of calling GLUpload.  
		// seeing artifacs when using GLUpload()
		// only possible because i know the format (GL_RG32F) ahead of time

		ktxTexture* kTexture;
		KTX_error_code errorCode;
		ktx_size_t offset;
		ktx_uint8_t* image;
		ktx_uint32_t level, layer, faceSlice;


		errorCode = ktxTexture_CreateFromNamedFile(filepath, KTX_TEXTURE_CREATE_ALLOC_STORAGE, &kTexture);
		if (errorCode != 0)
		{
			MNEMOSY_ERROR("KtxImage::LoadBrdfKTX - CreatFromNamedFile Failed \nError code: {}", ktxErrorString(errorCode));
			numChannels = 0; width = 0; height = 0;
			ktxTexture_Destroy(kTexture);
			return false;
		}


		level = 0; layer = 0; faceSlice = 0;
		errorCode = ktxTexture_GetImageOffset(kTexture, level, layer, faceSlice, &offset);
		if (errorCode != 0)
		{
			MNEMOSY_ERROR("KtxImage::LoadBrdfKTX - GetImageOffset Failed \nError code: {}", ktxErrorString(errorCode));
			numChannels = 0; width = 0; height = 0;
			ktxTexture_Destroy(kTexture);
			return false;
		}

		image = ktxTexture_GetData(kTexture) + offset;

		int res = kTexture->baseWidth;

		glGenTextures(1, &glTextureID);
		glBindTexture(GL_TEXTURE_2D, glTextureID);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RG32F, res, res, 0, GL_RG, GL_FLOAT, image);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		
		glActiveTexture(GL_TEXTURE0);
		glGenerateMipmap(GL_TEXTURE_2D);

		//delete image; // prob gets deleted by ktxTexture_destroy // causes crash
		ktxTexture_Destroy(kTexture);
		
		return true;
	}

	const bool KtxImage::LoadCubemap(const char* filepath, unsigned int& glTextureID, bool loadStoredMips, bool genMips) {

		namespace fs = std::filesystem;
		{
			// we do those checks here , even if they might be done early already because libKtx doesn't handle error very well and crashes easily.

			std::filesystem::path  p{ filepath };

			if (!fs::exists(p)) {
				MNEMOSY_ERROR("Ktx Load Cubemap Failed - path does not exsit. Path: {}", filepath);
				return false;
			}
			if (p.extension() != ".ktx2") {
				MNEMOSY_ERROR("Ktx Load Cubemap Failed - File is not a ktx file. Path: {}", filepath);
				return false;
			}
		}		
		
		ktxTexture2* kTexture = nullptr;
		KTX_error_code errorCode;

		//errorCode = ktxTexture_CreateFromNamedFile(filepath, KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, &kTexture);
		errorCode = ktxTexture2_CreateFromNamedFile(filepath, KTX_TEXTURE_CREATE_ALLOC_STORAGE, &kTexture);
		if (errorCode != 0) {

			MNEMOSY_ERROR("KtxImage::LoadCubemap: - CreatFromNamedFile Failed \nError code: {}", ktxErrorString(errorCode));
			numChannels = 0; width = 0; height = 0;

			if(kTexture)
				ktxTexture_Destroy( ktxTexture(kTexture) );

			return false;
		}

		graphics::TextureFormat mnsyFormat = GetMnemosyFormatFromVkFormat((VkFormat)kTexture->vkFormat);
		if (mnsyFormat == graphics::TextureFormat::MNSY_NONE) {
			MNEMOSY_ERROR("KtxImage::LoadCubemap: Faild. Unsupported texture format.");
			return false;
		}


		
		// Check if its a cubemap
		if (!kTexture->isCubemap ||  kTexture->numFaces != 6) {
			MNEMOSY_ERROR("Failed to load cubemap because ktx imgase is not a proper cubemap with 6 faces");
			if(kTexture)
				ktxTexture_Destroy(ktxTexture(kTexture));
			return false;
		}
		
		uint32_t resolution = kTexture->baseWidth; // we asume that cubemaps always have square faces

		width = resolution;
		height = resolution;

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, glTextureID);
		//glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

		uint8_t num_mips = kTexture->numLevels;

		
		if (!loadStoredMips) {
			// only load the first mip in this case
			num_mips = 1;
		}
				
		ktx_uint8_t* image; // idk how this works for higher bit depths but thats what ktxTexture_GetData() returns

		//ktx_uint32_t layer = 0; // layer is probably for when a ktx has multiple layers, sort of like photoshop layer or somthing
		ktx_size_t current_offset = 0;		

		uint32_t nextMipRes = resolution;

		// Get Element size only return bytesPerPixel for uncompressed textures. im only loading my own uncompress ktx files so it should be fine
		//uint32_t bytesPerPixel = ktxTexture_GetElementSize(ktxTexture(kTexture));
		
		uint8_t _numChannels, bitsPerChannel, bytesPerPixel;
		graphics::TexUtil::get_information_from_textureFormat(mnsyFormat, _numChannels, bitsPerChannel, bytesPerPixel);
		numChannels = _numChannels;
		
		for (ktx_uint32_t mip = 0; mip < num_mips; mip++) {

			// ensure by alignment is propely set for each mip.
			unsigned int unpackAlignment = 4;
			unsigned int rowSize = nextMipRes * bytesPerPixel;
			if (rowSize % 4 != 0) {
				unpackAlignment = 1;
			}
			glPixelStorei(GL_UNPACK_ALIGNMENT, unpackAlignment);

			for (ktx_uint32_t face = 0; face < 6; face++) {

				errorCode = ktxTexture_GetImageOffset(ktxTexture(kTexture), mip, 0, face, &current_offset);  // 0 here is for layers but we dont support it.
				if (errorCode != 0) {
					MNEMOSY_ERROR("KtxImage::LoadCubeKTX - GetImageOffset Failed \nError code: {}", ktxErrorString(errorCode));
					numChannels = 0; width = 0; height = 0;

					if (kTexture) {
						ktxTexture_Destroy(ktxTexture(kTexture));
					}
					return false;
				}

				image = ktxTexture_GetData(ktxTexture(kTexture)) + current_offset;

				// upload to gl
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, mip, GL_RGB32F, nextMipRes, nextMipRes, 0, GL_RGB, GL_FLOAT, image);

			}

			/*if (nextMipRes <= 2) // this fails for some reason
				break;*/

			nextMipRes = nextMipRes / 2;

		}

		// can destroy this now
		ktxTexture_Destroy(ktxTexture(kTexture));

		//// set filter options
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, glTextureID);

		if (genMips && !loadStoredMips) {
			glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
		}

		return true;
	}

	const bool KtxImage::SaveCubemap(const char* filepath, unsigned int& glTextureID, TextureFormat format, unsigned int resolution, bool storeMipMaps) {

		ktxTexture2* texture;
		ktxTextureCreateInfo createInfo;
		KTX_error_code errorCode;

		// lets store cubemaps always like this
		createInfo.glInternalformat = GL_RGB32F;
		createInfo.vkFormat = VK_FORMAT_R32G32B32_SFLOAT; // Idialy this -> VK_FORMAT_BC6H_SFLOAT_BLOCK  so using -> VK_FORMAT_R32G32B32_SFLOAT works atm
		createInfo.baseWidth = resolution;
		createInfo.baseHeight = resolution;
		createInfo.baseDepth = 1; // ignoring 3d textures for now
		createInfo.numDimensions = 2; // 2d Texture  // 3= cubemap

		// Number of mipmaps
		createInfo.numLayers = 1; // num of array elements should be 0 no array
		createInfo.numFaces = 6; // must be 6 if cubemap
		createInfo.isArray = KTX_FALSE;
		
		if (storeMipMaps) {
			createInfo.generateMipmaps = KTX_FALSE;
			createInfo.numLevels = (ktx_uint32_t)log2(createInfo.baseWidth) + 1;
		}
		else {
			createInfo.generateMipmaps = KTX_TRUE;
			createInfo.numLevels = 1;
		}

		errorCode = ktxTexture2_Create(&createInfo, KTX_TEXTURE_CREATE_ALLOC_STORAGE, &texture);
		if (errorCode != 0) {
			MNEMOSY_ERROR("KtxImage::SaveCubemapKtx: Create Texture Failed \nError code: {}", ktxErrorString(errorCode));
			ktxTexture_Destroy(ktxTexture(texture));
			return false;
		}
		//ktx_uint32_t layer = 0; // array layer but I dont use it

		// for cubemaps only allow rgb channels
		uint8_t channels = 3;
		uint8_t mip = 0;

		uint16_t bytesOfPixel = (sizeof(float)) * channels ;// bytes of one pixel
		uint16_t baseRes = resolution;
		uint16_t nextMipRes = baseRes;

		glActiveTexture(GL_TEXTURE0); // just to make sure
		glBindTexture(GL_TEXTURE_CUBE_MAP, glTextureID);

		// ensure we are getting 4 byte aligned data from openGl
		glPixelStorei(GL_PACK_ALIGNMENT, 4);

		void* buf = malloc(baseRes* baseRes * channels * sizeof(float));

		for (unsigned int mip = 0; mip < (unsigned int)createInfo.numLevels; mip++) {
			//float* pixels = new float[nextMipRes * nextMipRes * channels];
			size_t srcSize = (nextMipRes * nextMipRes) * bytesOfPixel;

			// loop over cubemap faces
			for (uint8_t cubeFace = 0; cubeFace < 6; cubeFace++) {
				
				glGetTexImage(GL_TEXTURE_CUBE_MAP_POSITIVE_X + cubeFace , mip, GL_RGB, GL_FLOAT, buf);
				
				errorCode = ktxTexture_SetImageFromMemory(ktxTexture(texture), mip, 0 , cubeFace, (unsigned char*)buf, srcSize);
				if (errorCode != 0) {
					MNEMOSY_ERROR("KtxImage::SaveCubemapKtx: SetImageFromMemory Failed \nError code: {}", ktxErrorString(errorCode));
					ktxTexture_Destroy(ktxTexture(texture));
					
					if (buf) {
						free(buf);
					}
					
					return false;
				}
			}
			
			// if it becomes < 2 we cant devide anymore and it causes frees on the math operation
			if (nextMipRes <= 2) 
				break;

			nextMipRes = (unsigned int)nextMipRes / 2;
		}

		if (buf) {
			free(buf);
		}

		errorCode = ktxTexture_WriteToNamedFile(ktxTexture(texture), filepath);
		if (errorCode != 0)
		{
			MNEMOSY_ERROR("KtxImage::SaveCubemapKtx: WriteToNamedFile Failed \nError code: {}", ktxErrorString(errorCode));
			ktxTexture_Destroy(ktxTexture(texture));
			return false;
		}

		
		
		ktxTexture_Destroy(ktxTexture(texture));
		
		
		return true;
	}
	
	
	
	const bool KtxImage::SaveKtx(const char* filepath,unsigned char* imageData, unsigned int numChannels, unsigned int width, unsigned int height) {

		ktxTexture2* texture;                   // For KTX2
		ktxTextureCreateInfo createInfo;
		KTX_error_code errorCode;

		// glInternalFormat is ignored when creating a ktxTexture2 visVersa with vkFormat and ktxTexture1
		// _SRGB for srgb _UNORM for linear images
		if (numChannels == 4) {
			createInfo.glInternalformat = GL_RGBA8;
			createInfo.vkFormat = VK_FORMAT_R8G8B8A8_UNORM;
		}
		if (numChannels == 3) {
			createInfo.glInternalformat = GL_RGB8;
			createInfo.vkFormat = VK_FORMAT_R8G8B8_UNORM;
		}
		if (numChannels == 2) // NOT TESTED
		{
			createInfo.glInternalformat = GL_RG16;
			createInfo.vkFormat = VK_FORMAT_R16G16_SFLOAT; // might have to use VK_FORMAT_R16G16_UNORM
		}
		if (numChannels == 1) // works but so far only with r8 not yet R16 but probably because of source files not having correct format
		{
			createInfo.glInternalformat = GL_R8; 
			createInfo.vkFormat = VK_FORMAT_R8_UNORM; // might have to use VK_FORMAT_R16_UNORM  VK_FORMAT_R16_SFLOAT
		}

		createInfo.baseWidth = width;
		createInfo.baseHeight = height;
		createInfo.baseDepth = 1; // ignoring 3d textures for now
		createInfo.numDimensions = 2; // 2d Texture  // 3= cubemap
		
		// Number of mipmaps
		createInfo.numLevels = (ktx_uint32_t)log2(createInfo.baseWidth) + 1;
		createInfo.numLayers = 1; // num of array elements should be 0 no array
		createInfo.numFaces = 1; // must be 6 if cubemap
		createInfo.isArray = KTX_FALSE;
		createInfo.generateMipmaps = KTX_FALSE;
		
		errorCode = ktxTexture2_Create(&createInfo,KTX_TEXTURE_CREATE_ALLOC_STORAGE,&texture);
		if (errorCode != 0)	{
			MNEMOSY_ERROR("KtxImage::SaveKtx: Create Texture Failed \nError code: {}", ktxErrorString(errorCode));
			ktxTexture_Destroy(ktxTexture(texture));
			return false;
		}

		ktx_uint32_t level = 0;
		ktx_uint32_t layer = 0;
		ktx_uint32_t faceSlice = 0;

		// might have to change this if saving RG or R channels as 16bitfloats or whatever
		ktx_size_t charSize = sizeof(unsigned char); 
		ktx_size_t srcSize = (width * height) * (charSize * numChannels);



		errorCode = ktxTexture_SetImageFromMemory(ktxTexture(texture), layer, layer, faceSlice, imageData, srcSize);
		if (errorCode != 0)
		{
			MNEMOSY_ERROR("KtxImage::SaveKtx: SetImageFromMemory Failed \nError code: {}",ktxErrorString(errorCode));
			ktxTexture_Destroy(ktxTexture(texture));
			return false;
		}

		// here should be code to write mip maps to the file aswell // prob also for cubemap 
			// Repeat for the other 15 slices of the base level and all other levels
			// up to createInfo.numLevels.




		errorCode = ktxTexture_WriteToNamedFile(ktxTexture(texture), filepath);
		if (errorCode != 0)	{

			//MNEMOSY_ASSERT(false, "");
			MNEMOSY_ERROR("KtxImage::SaveKtx: WriteToNamedFile Failed \nError code: {}", ktxErrorString(errorCode));
			ktxTexture_Destroy(ktxTexture(texture));
			return false;
		}

		ktxTexture_Destroy(ktxTexture(texture));
		return true;
	}

	const bool KtxImage::SaveBrdfLutKtx(const char* filepath, unsigned int& glTextureID, unsigned int resolution)
	{
		ktxTexture2* texture;
		ktxTextureCreateInfo createInfo;
		KTX_error_code errorCode;

		// lets store brdf always like this
		createInfo.glInternalformat = GL_RG32F;
		createInfo.vkFormat = VK_FORMAT_R32G32_SFLOAT;

		createInfo.baseWidth = resolution;
		createInfo.baseHeight = resolution;
		createInfo.baseDepth = 1; // ignoring 3d textures for now
		createInfo.numDimensions = 2; // 2d Texture

		// Number of mipmaps
		createInfo.numLevels = (ktx_uint32_t)log2(createInfo.baseWidth) + 1;
		createInfo.numLayers = 1; // num of array elements should be 0 no array
		createInfo.numFaces = 1; // must be 6 if cubemap
		createInfo.isArray = KTX_FALSE;
		createInfo.generateMipmaps = KTX_FALSE;

		errorCode = ktxTexture2_Create(&createInfo, KTX_TEXTURE_CREATE_ALLOC_STORAGE, &texture);
		if (errorCode != 0)
		{
			MNEMOSY_ERROR("KtxImage::SaveCubemapKtx: Create Texture Failed \nError code: {}", ktxErrorString(errorCode));
			ktxTexture_Destroy(ktxTexture(texture));
			return false;
		}

		ktx_uint32_t layer = 0;
		ktx_uint32_t faceSlice = 0;

		int channels = 2;
		ktx_size_t bytesOfPixel = sizeof(float) * channels;// bytes of one pixel
		
		int mip = 0;
		int baseRes = resolution;

		int nextMipRes = baseRes;

		glActiveTexture(GL_TEXTURE0); // just to make sure
		glBindTexture(GL_TEXTURE_2D, glTextureID);

		// loop over mipmaps
		for (int mip = 0; mip < (int)createInfo.numLevels; mip++)
		{
			float* pixels = new float[nextMipRes * nextMipRes * channels];
			ktx_size_t srcSize = (nextMipRes * nextMipRes) * bytesOfPixel;
			
			glGetTexImage(GL_TEXTURE_2D, mip, GL_RG, GL_FLOAT, pixels);
			
			
			errorCode = ktxTexture_SetImageFromMemory(ktxTexture(texture), mip, layer, faceSlice, (unsigned char*)pixels, srcSize);
			if (errorCode != 0)
			{
				MNEMOSY_ERROR("KtxImage::SaveCubemapKtx: SetImageFromMemory Failed \nError code: {}", ktxErrorString(errorCode));
				ktxTexture_Destroy(ktxTexture(texture));

				delete[] pixels;
				pixels = nullptr;

				return false;
			}

			delete[] pixels;
			pixels = nullptr;

			// if it becomes < 2 we cant devide anymore and it causes frees on the math operation
			if (nextMipRes <= 2)
				break;

			nextMipRes = (int)round((double)nextMipRes / 2);
		}

		errorCode = ktxTexture_WriteToNamedFile(ktxTexture(texture), filepath);
		if (errorCode != 0)
		{
			MNEMOSY_ERROR("KtxImage::SaveKtx: WriteToNamedFile Failed \nError code: {}", ktxErrorString(errorCode));
			ktxTexture_Destroy(ktxTexture(texture));
			return false;
		}

		ktxTexture_Destroy(ktxTexture(texture));
		return true;

	}


	//fixme please
	const bool KtxImage::ExportGlTexture(const char* filepath, unsigned int glTextureID, const unsigned int numChannels, const unsigned int width, const unsigned int height, ktxImgFormat imgFormat, bool exportMips) {

		ktxTexture2* texture;
		ktxTextureCreateInfo createInfo;
		KTX_error_code errorCode;
		ktx_size_t bytesOfPixel; // bytes of one pixel
		int channels = 4;

		if (imgFormat == MNSY_COLOR) {

			if (numChannels == 4) {
				createInfo.glInternalformat = GL_RGBA8;
				createInfo.vkFormat = VK_FORMAT_R8G8B8A8_UNORM; // this should really be srgb but it works rn, just the output texture is in linear spaceVK_FORMAT_R8G8B8_SNORM
				channels = 4;
				bytesOfPixel = (sizeof(ktx_uint8_t) * 4);
			}
			else if (numChannels == 3) {
				createInfo.glInternalformat = GL_RGB8;
				createInfo.vkFormat = VK_FORMAT_R8G8B8_UNORM;
				channels = 3;
				bytesOfPixel = (sizeof(ktx_uint8_t) * 3);
			}
			else if (numChannels == 1) {
				createInfo.glInternalformat = GL_RGB8;
				createInfo.vkFormat = VK_FORMAT_R8G8B8_UNORM;
				channels = 3;
				bytesOfPixel = (sizeof(ktx_uint8_t) * 3);


				//MNEMOSY_ERROR("You cannot export this texture as a color map because it only has one channel.");
				//return false;
			}
			else {
				MNEMOSY_ERROR("You cannot use this texture because it has a weird amount of channels: {}", numChannels);
				return false;
			}

		}

		else if (imgFormat == MNSY_COLOR_SRGB) {

			if (numChannels == 4) {
				createInfo.glInternalformat = GL_RGBA8;
				createInfo.vkFormat = VK_FORMAT_R8G8B8A8_SRGB;
				channels = 4;
				bytesOfPixel = (sizeof(ktx_uint8_t) * 4);
			}
			else if (numChannels == 3) {
				createInfo.glInternalformat = GL_RGB8;
				createInfo.vkFormat = VK_FORMAT_R8G8B8_SRGB;
				channels = 3;
				bytesOfPixel = (sizeof(ktx_uint8_t) * 3);
			}
			else if (numChannels == 1) { // this should not happen
				createInfo.glInternalformat = GL_RGB8;
				createInfo.vkFormat = VK_FORMAT_R8G8B8_SRGB;
				channels = 3;
				bytesOfPixel = (sizeof(ktx_uint8_t) * 3);

				//MNEMOSY_ERROR("You cannot export this texture as a color map because it only has one channel.");
				//return false;
			}
		}


		else if (imgFormat == MNSY_NORMAL) {
			createInfo.glInternalformat = GL_RGB16F;
			createInfo.vkFormat = VK_FORMAT_R16G16B16_SFLOAT;
			channels = 3;
			bytesOfPixel = (sizeof(short) * 3);
		}

		else if (imgFormat == MNSY_LINEAR_CHANNEL) {
			createInfo.glInternalformat = GL_R16F;
			createInfo.vkFormat = VK_FORMAT_R16_SFLOAT;
			channels = 1;
			bytesOfPixel = (sizeof(short) * 1);
		}

		createInfo.baseWidth = width;
		createInfo.baseHeight = height;
		createInfo.baseDepth = 1; // ignoring 3d textures for now
		createInfo.numDimensions = 2; // 2d Texture  // 3 = cubemap

		// Number of mipmaps
		if (exportMips) {
			// Force Mip export always off for now because there is issue with some texture not able to export on lower mips
			createInfo.numLevels = 1;
			createInfo.generateMipmaps = KTX_TRUE;
			//createInfo.numLevels = (ktx_uint32_t)log2(createInfo.baseWidth) + 1;  
			//createInfo.generateMipmaps = KTX_FALSE;
		}
		else {
			createInfo.numLevels = 1;
			createInfo.generateMipmaps = KTX_TRUE;
		}


		createInfo.numLayers = 1; // num of array elements should be 0 no array
		createInfo.numFaces = 1; // must be 6 if cubemap
		createInfo.isArray = KTX_FALSE;		
		errorCode = ktxTexture2_Create(&createInfo, KTX_TEXTURE_CREATE_ALLOC_STORAGE, &texture);
		if (errorCode != 0)
		{
			MNEMOSY_ERROR("KtxImage::ExportGlTexture: Create Texture Failed \nError code: {}", ktxErrorString(errorCode));
			ktxTexture_Destroy(ktxTexture(texture));
			return false;
		}

		ktx_uint32_t layer = 0; // array layer but I dont use it

		glActiveTexture(GL_TEXTURE0); // just to make sure
		glBindTexture(GL_TEXTURE_2D, glTextureID);
		
		int nextMip_Width  = createInfo.baseWidth;
		int nextMip_Height = createInfo.baseHeight;
		float InitialAspectRatio = (float)nextMip_Width / (float)nextMip_Height;;

		// TODO: At the moment this method crashes when exporting lower mips, especially with non square aspect ratios.
		// this likely has to do with some rounding issues when it gets to a point and the aspect ratio becomes different it allocates the wrong amount of bytes.
		// for now exporting mipmaps is compleatly disables and we always generate them new on import.
		if (imgFormat == MNSY_COLOR || imgFormat == MNSY_COLOR_SRGB) {
			for (int mip = 0; mip < (int)createInfo.numLevels; mip++) {

				// break because we cant go lower thant 2x2 pixels
				if (nextMip_Width <= 2 || nextMip_Height <= 2)
					break;

				ktx_size_t mipSizeBytes = nextMip_Width * nextMip_Height * channels * sizeof(uint8_t);


				void* pixels = malloc(mipSizeBytes);

				glPixelStorei(GL_PACK_ALIGNMENT, 4); // should be dependent on width and pixel row byte alingment but works for thumbnails 256x256 for now
				// RGB or RGBA texture
				// get pixel data from a glUploadedTexture
				if (channels == 4) {
					glGetTexImage(GL_TEXTURE_2D, mip, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
				}
				else if (channels == 3) {
					glGetTexImage(GL_TEXTURE_2D, mip, GL_RGB, GL_UNSIGNED_BYTE, pixels);
				}

				errorCode = ktxTexture_SetImageFromMemory(ktxTexture(texture), mip, layer, 0, (unsigned char*)pixels, mipSizeBytes);
				if (errorCode != 0) {
					MNEMOSY_ERROR("KtxImage::ExportGlTexture: SetImageFromMemory Failed \nError code: {}", ktxErrorString(errorCode));
					ktxTexture_Destroy(ktxTexture(texture));

					free(pixels);
					return false;
				}
				free(pixels);
							

				nextMip_Width  = (int)((double)nextMip_Width * 0.5f);
				nextMip_Height = (int)((double)nextMip_Height * 0.5f);

			}
		}
		else if (imgFormat == MNSY_NORMAL) {
			// loop over mipmaps
			for (int mip = 0; mip < (int)createInfo.numLevels; mip++) {

				if (nextMip_Width <= 2 || nextMip_Height <= 2)
					break;
				
				ktx_size_t mipSizeBytes = nextMip_Width * nextMip_Height * channels * sizeof(ktx_uint16_t);

				float aspectRatio = (float)nextMip_Width / (float)nextMip_Height;
				MNEMOSY_TRACE("ExportingMip: {}, Width: {}	Height: {}	Channels: {}	Bytes: {}	Aspect Ratio: {}", mip, nextMip_Width, nextMip_Height, channels, (float)mipSizeBytes, aspectRatio);

					
				ktx_uint16_t* pixels = new ktx_uint16_t[mipSizeBytes];

				glGetTexImage(GL_TEXTURE_2D, mip, GL_RGB, GL_HALF_FLOAT, pixels);
				errorCode = ktxTexture_SetImageFromMemory(ktxTexture(texture), mip, layer, 0, (unsigned char*)pixels, mipSizeBytes);
				if (errorCode != 0) {

					MNEMOSY_ERROR("KtxImage::ExportGLTexture: SetImageFromMemory Failed \nError code: {}", ktxErrorString(errorCode));
					ktxTexture_Destroy(ktxTexture(texture));
					delete[] pixels;
					return false;
				}
				delete[] pixels;

				nextMip_Width	= (int)((double)nextMip_Width  * 0.5f);
				nextMip_Height	= (int)((double)nextMip_Height * 0.5f);
			}
		}
		else if (imgFormat == MNSY_LINEAR_CHANNEL) {
			// loop over mipmaps
			for (int mip = 0; mip < (int)createInfo.numLevels; mip++) {

				// if it becomes < 2 we cant devide anymore
				if (nextMip_Width <= 2 || nextMip_Height <= 2)
					break;
				
				ktx_size_t mipSizeBytes = nextMip_Width * nextMip_Height * channels * sizeof(ktx_uint16_t);
				ktx_uint16_t* pixels = new ktx_uint16_t[mipSizeBytes];

				glGetTexImage(GL_TEXTURE_2D, mip, GL_RED, GL_HALF_FLOAT, pixels);

				errorCode = ktxTexture_SetImageFromMemory(ktxTexture(texture), mip, layer, 0, (unsigned char*)pixels, mipSizeBytes);
				if (errorCode != 0) {
					MNEMOSY_ERROR("KtxImage::ExportGLTexure: SetImageFromMemory Failed \nError code: {}", ktxErrorString(errorCode));
					ktxTexture_Destroy(ktxTexture(texture));
					delete[] pixels;
					return false;
				}
				delete[] pixels;

				// Calculate next mip size
				nextMip_Width  = (int)((double)nextMip_Width  * 0.5f);
				nextMip_Height = (int)((double)nextMip_Height * 0.5f);
			}
		}

		// export to file
		errorCode = ktxTexture_WriteToNamedFile(ktxTexture(texture), filepath);
		if (errorCode != 0) {
			MNEMOSY_ERROR("KtxImage::ExportGLTexture: WriteToNamedFile Failed \nError code: {}", ktxErrorString(errorCode));
			ktxTexture_Destroy(ktxTexture(texture));
			return false;
		}

		ktxTexture_Destroy(ktxTexture(texture));
		return true;
	}


	unsigned int KtxImage::Save_WithoutMips(const char* filepath, void* pixels,const bool flipVertically, const TextureFormat format, const uint16_t _width, const uint16_t _height, const bool isHalfFloat)
	{
		if (pixels == nullptr) {
			return (unsigned int)KTX_FILE_DATA_ERROR;
		}

		if (format == graphics::TextureFormat::MNSY_NONE) {
			return (unsigned int)KTX_INVALID_VALUE;
		}

		uint8_t _numChannels, _bitsPerChannel, _bytesPerPixel;

		graphics::TexUtil::get_information_from_textureFormat(format, _numChannels, _bitsPerChannel, _bytesPerPixel);

		size_t bufferSize = (size_t)_width * _height * _bytesPerPixel;

		// perform flip in memory if nessesary.
		void* buffer = pixels;

		
		bool flipUsingScanline = flipVertically;

		// this is disabled for now bc we just export with unorm.
		// 
		// check if we can perform a fast flip using scanline approach which is only the case if we dont need to convert unsign short 16bit data to half float 
		//bool flipUsingScanline = false;
		//if (flipVertically) {
		//	if (_bitsPerChannel == 16) {
		//		if (isHalfFloat) {
		//			flipUsingScanline = true;
		//		}
		//	}
		//	else {
		//		flipUsingScanline = true;
		//	}
		//}


		if (flipUsingScanline) {

			buffer = nullptr;
			buffer = malloc(bufferSize);

			size_t sizePerRow = (size_t)_width * _bytesPerPixel;

			for (uint16_t h = 0; h < _height; h++) {


				size_t offsetSrc = (_height - h - 1) * _width * _numChannels;
				size_t offsetDest = h * _width * _numChannels;


				if (_bitsPerChannel == 8) {
					memcpy((uint8_t*)buffer + offsetDest, (uint8_t*)pixels + offsetSrc, sizePerRow);
				}
				else if (_bitsPerChannel == 16) {
					memcpy((uint16_t*)buffer + offsetDest, (uint16_t*)pixels + offsetSrc, sizePerRow);
				}
				else if (_bitsPerChannel == 32) {
					memcpy((uint32_t*)buffer + offsetDest, (uint32_t*)pixels + offsetSrc, sizePerRow);
				}
			}
		}
		

		// we want to always convert 16bit unsigned short data to half float data. this shit is broken...
		bool convert16bituIntToHalfFloat = false;// _bitsPerChannel == 16 && !isHalfFloat;

		//if (convert16bituIntToHalfFloat) {


		//	buffer = nullptr;
		//	buffer = malloc(bufferSize);


		//	for (uint16_t h = 0; h < _height; h++) {

		//		for (uint16_t w = 0; w < _width; w++) {

		//			// asume we flip vertically
		//			uint16_t y = _height - h - 1;
		//			if (!flipVertically) {
		//				y = h;
		//			}

		//			for (uint16_t c = 0; c < _numChannels; c++) {


		//				size_t offsetSrc = y * w * _numChannels + c;
		//				size_t offsetDest = h * w * _numChannels + c;

		//				uint16_t bits = *((uint16_t*)pixels + offsetSrc);

		//				float floatNormalized = static_cast<float>(bits) / (static_cast<float>(UINT16_MAX));

		//				// we are using openExr half float type.
		//				Imath::half value = Imath::half(floatNormalized);

		//				memcpy((uint16_t*)buffer + offsetDest, &value, sizeof(uint16_t));
		//			}
		//		}

		//	}
		//	//// swap endianess
		//		//size_t pixelCount = _width * _height * _numChannels;

		//		//for (size_t i = 0; i < pixelCount; ++i) {
		//		//	// The pointer to the current 16-bit pixel
		//		//	unsigned char* pixel = (unsigned char*)buffer + i * 2;
		//		//	// Swap the two bytes to convert from big-endian to little-endian
		//		//	unsigned char temp = pixel[0];
		//		//	pixel[0] = pixel[1];
		//		//	pixel[1] = temp;
		//		//}
		//}

		ktxTexture2* kTexture;
		ktxTextureCreateInfo createInfo;
		KTX_error_code errorCode;
		
		//createInfo.glInternalformat = GL_RGB32F;
		createInfo.vkFormat = GetVkFormatFromMnemosyFormat(format,isHalfFloat);

		createInfo.baseWidth = _width;
		createInfo.baseHeight = _height;
		createInfo.baseDepth = 1; // ignoring 3d textures for now
		createInfo.numDimensions = 2; // 2d Texture  // 3= cubemap

		// Number of mipmaps
		createInfo.numLayers = 1; // num of array elements should be 0 no array
		createInfo.numFaces = 1;	// no cubemap
		createInfo.isArray = KTX_FALSE;
		createInfo.generateMipmaps = KTX_TRUE;
		createInfo.numLevels = 1; // numMips
			

		errorCode = ktxTexture2_Create(&createInfo, KTX_TEXTURE_CREATE_ALLOC_STORAGE, &kTexture);
		if (errorCode != 0) {

			if (kTexture) {
				ktxTexture_Destroy(ktxTexture(kTexture));
			}
			
			return (unsigned int)errorCode;
		}


		errorCode = ktxTexture_SetImageFromMemory(ktxTexture(kTexture), 0, 0, 0, (unsigned char*)buffer, bufferSize);
		if (errorCode != 0) {
						
			if (kTexture) {
				ktxTexture_Destroy(ktxTexture(kTexture));
			}

			return (unsigned int)errorCode;
		}


		errorCode = ktxTexture_WriteToNamedFile(ktxTexture(kTexture), filepath);
		if (errorCode != 0)
		{
			if (kTexture) {
				ktxTexture_Destroy(ktxTexture(kTexture));
			}

			return (unsigned int)errorCode;
		}

		if (kTexture) {
			ktxTexture_Destroy(ktxTexture(kTexture));
		}

		if (flipUsingScanline || convert16bituIntToHalfFloat) {

			if (buffer) {
				free(buffer);
			}
		}

		return (unsigned int)KTX_SUCCESS;
	}

	TextureFormat KtxImage::GetMnemosyFormatFromVkFormat(VkFormat vkFormat) {
		switch (vkFormat)
		{
		case VK_FORMAT_R8_UNORM:		return TextureFormat::MNSY_R8;
		case VK_FORMAT_R8G8_UNORM:		return TextureFormat::MNSY_RG8;
		case VK_FORMAT_R8G8B8_UNORM:	return TextureFormat::MNSY_RGB8;
		case VK_FORMAT_R8G8B8A8_UNORM:	return TextureFormat::MNSY_RGBA8;

		case VK_FORMAT_R16_UNORM:			return TextureFormat::MNSY_R16;
		case VK_FORMAT_R16G16_UNORM:		return TextureFormat::MNSY_RG16;
		case VK_FORMAT_R16G16B16_UNORM:		return TextureFormat::MNSY_RGB16;
		case VK_FORMAT_R16G16B16A16_UNORM:	return TextureFormat::MNSY_RGBA16;

		// for future
		//case VK_FORMAT_R16_SFLOAT:			return TextureFormat::MNSY_NONE;
		//case VK_FORMAT_R16G16_SFLOAT:		return TextureFormat::MNSY_NONE;
		//case VK_FORMAT_R16G16B16_SFLOAT:	return TextureFormat::MNSY_NONE;
		//case VK_FORMAT_R16G16B16A16_SFLOAT: return TextureFormat::MNSY_NONE;

		case VK_FORMAT_R32_SFLOAT:				return TextureFormat::MNSY_R32;
		case VK_FORMAT_R32G32_SFLOAT:			return TextureFormat::MNSY_RG32;
		case VK_FORMAT_R32G32B32_SFLOAT:		return TextureFormat::MNSY_RGB32;
		case VK_FORMAT_R32G32B32A32_SFLOAT:		return TextureFormat::MNSY_RGBA32;
		default:
			return TextureFormat::MNSY_NONE;
			break;
		}

		return TextureFormat::MNSY_NONE;
	}

	VkFormat KtxImage::GetVkFormatFromMnemosyFormat(TextureFormat mnsyFormat, bool isHalfFloat) {
		unsigned int vkFormat = 0;

		switch (mnsyFormat)
		{
		case mnemosy::graphics::MNSY_NONE:		vkFormat = 0;	break;

		case mnemosy::graphics::MNSY_R8:		vkFormat = 9;	break;
		case mnemosy::graphics::MNSY_RG8:		vkFormat = 16;	break;
		case mnemosy::graphics::MNSY_RGB8:		vkFormat = 23;	break;
		case mnemosy::graphics::MNSY_RGBA8:		vkFormat = 37;	break;

		case mnemosy::graphics::MNSY_R16:		
			if (isHalfFloat) {
				vkFormat = 76;
			}
			else {
				vkFormat = 70;	
			}
			break;
		case mnemosy::graphics::MNSY_RG16:		
			if (isHalfFloat) {
				vkFormat = 83;	
			}
			else {
				vkFormat = 77;	
			}
			break;
		case mnemosy::graphics::MNSY_RGB16:
			if (isHalfFloat) {
				vkFormat = 90;// VK_FORMAT_R16G16B16_SFLOAT
			}
			else {
				vkFormat = 84; // UNORM
				//vkFormat = 88;// _UINT
			}
			break;
		case mnemosy::graphics::MNSY_RGBA16:	
			if (isHalfFloat) {
				vkFormat = 97;
			}
			else {
				vkFormat = 91;
			}
			break;
												
		case mnemosy::graphics::MNSY_R32:		vkFormat = 100; break;
		case mnemosy::graphics::MNSY_RG32:		vkFormat = 103; break;
		case mnemosy::graphics::MNSY_RGB32:		vkFormat = 106; break;
		case mnemosy::graphics::MNSY_RGBA32:	vkFormat = 109; break;
		default:
			break;
		}

		return (VkFormat)vkFormat;
	}

} // mnemosy::graphics