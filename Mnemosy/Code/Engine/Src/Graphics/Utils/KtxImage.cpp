#include "Include/Graphics/Utils/KtxImage.h"
#include "Include/Core/Log.h"

#include <glad/glad.h>
#include <ktx.h>
#include <vkformat_enum.h>

#include <stdio.h>
#include <math.h>

namespace mnemosy::graphics
{
	bool KtxImage::LoadKtx(const char* filepath, unsigned int& glTextureID)
	{
		{
			ktxTexture* kTexture;
			KTX_error_code errorCode;
			
			errorCode = ktxTexture_CreateFromNamedFile(filepath, KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, &kTexture);
			if (errorCode != 0)
			{
				MNEMOSY_ERROR("KtxImage::LoadKtx - CreatFromNamedFile Failed \nError code: {}", ktxErrorString(errorCode));
				numChannels = 0; width = 0; height = 0;
				ktxTexture_Destroy(kTexture);
				return false;
			}			


			GLenum target, glerror;
			errorCode = ktxTexture_GLUpload(ktxTexture(kTexture), &glTextureID, &target, &glerror);
			if (errorCode != 0)
			{
				MNEMOSY_ERROR("KtxImage::LoadKtx: GLUpload Failed \nError code: {}", ktxErrorString(errorCode));
				numChannels = 0; width = 0; height = 0;
				ktxTexture_Destroy(kTexture);
				return false;
			}

			ktxTexture_Destroy(kTexture);
			return true;
		}
	}

	bool KtxImage::LoadBrdfKTX(const char* filepath, unsigned int& glTextureID)
	{
		// function upload the image directly to openGl instead of calling GLUpload.  
		// seeing artifacs when using GLUpload()
		// only possible because i know the format (GL_RG32F) ahead of time

		ktxTexture* kTexture;
		KTX_error_code errorCode;
		ktx_size_t offset;
		ktx_uint8_t* image;
		ktx_uint32_t level, layer, faceSlice;


		errorCode = ktxTexture_CreateFromNamedFile(filepath, KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, &kTexture);
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

	bool KtxImage::SaveKtx(const char* filepath, unsigned char* imageData, unsigned int numChannels, unsigned int width, unsigned int height)
	{
		ktxTexture2* texture;                   // For KTX2
		ktxTextureCreateInfo createInfo;
		KTX_error_code errorCode;


		// glInternalFormat is ignored when creating a ktxTexture2 visVersa with vkFormat and ktxTexture1
		// _SRGB for srgb _UNORM for linear images
		if (numChannels == 4)
		{
			createInfo.glInternalformat = GL_RGBA8;
			createInfo.vkFormat = VK_FORMAT_R8G8B8A8_UNORM;
		}
		if (numChannels == 3)
		{
			createInfo.glInternalformat = GL_RGB8;
			createInfo.vkFormat = VK_FORMAT_R8G8B8_UNORM;
		}
		if (numChannels == 2) // NOT TESTED
		{
			createInfo.glInternalformat = GL_RG8;
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
		if (errorCode != 0)
		{
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
		if (errorCode != 0)
		{
			MNEMOSY_ERROR("KtxImage::SaveKtx: WriteToNamedFile Failed \nError code: {}", ktxErrorString(errorCode));
			ktxTexture_Destroy(ktxTexture(texture));
			return false;
		}

		ktxTexture_Destroy(ktxTexture(texture));
		return true;
	}

	bool KtxImage::SaveBrdfLutKtx(const char* filepath, unsigned int& glTextureID, unsigned int resolution)
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

	bool KtxImage::SaveCubemapKtx(const char* filepath, unsigned int& glTextureID, unsigned int resolution)
	{
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
		createInfo.numLevels = (ktx_uint32_t)log2(createInfo.baseWidth) + 1;
		createInfo.numLayers = 1; // num of array elements should be 0 no array
		createInfo.numFaces = 6; // must be 6 if cubemap
		createInfo.isArray = KTX_FALSE;
		createInfo.generateMipmaps = KTX_FALSE;

		errorCode = ktxTexture2_Create(&createInfo, KTX_TEXTURE_CREATE_ALLOC_STORAGE, &texture);
		if (errorCode != 0)
		{
			MNEMOSY_ERROR("KtxImage::SaveCubemapKtx: Create Texture Failed \nError code: {}", ktxErrorString(errorCode));
			ktxTexture_Destroy(ktxTexture(texture));
			return false;
		}

		ktx_uint32_t layer = 0; // array layer but I dont use it

		// for cubemaps only allow rgb channels
		int channels = 3;
		ktx_size_t bytesOfPixel = (sizeof(float)) * channels ;// bytes of one pixel


		int mip = 0;
		int baseRes = resolution;

		int nextMipRes = baseRes;

		glActiveTexture(GL_TEXTURE0); // just to make sure
		glBindTexture(GL_TEXTURE_CUBE_MAP, glTextureID);

		// loop over mipmaps
		for (int mip = 0; mip < (int)createInfo.numLevels; mip++)
		{
			float* pixels = new float[nextMipRes * nextMipRes * channels];
			ktx_size_t srcSize = (nextMipRes * nextMipRes) * bytesOfPixel;

			// loop over cubemap faces
			for (int cubeFace = 0; cubeFace < 6; cubeFace++)
			{
				glGetTexImage(GL_TEXTURE_CUBE_MAP_POSITIVE_X + cubeFace , mip, GL_RGB, GL_FLOAT, pixels);

				errorCode = ktxTexture_SetImageFromMemory(ktxTexture(texture), mip, layer, cubeFace,(unsigned char*)pixels, srcSize);
				if (errorCode != 0)
				{
					MNEMOSY_ERROR("KtxImage::SaveCubemapKtx: SetImageFromMemory Failed \nError code: {}", ktxErrorString(errorCode));
					ktxTexture_Destroy(ktxTexture(texture));
					
					delete[] pixels;
					pixels = nullptr;

					return false;
				}
			}

			
			delete [] pixels;
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

} // mnemosy::graphics