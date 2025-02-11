#include "Include/Graphics/Texture.h"
#include "Include/Core/Log.h"

#include "Include/Core/Utils/StringUtils.h"
#include "Include/Graphics/TextureDefinitions.h"
#include "Include/Graphics/Utils/KtxImage.h"
#include "Include/Graphics/Utils/Picture.h"

#include <glad/glad.h>
#include <math.h>
#include <filesystem>


namespace mnemosy::graphics
{
	Texture::Texture() {
		m_textureFormat = TextureFormat::MNSY_NONE;
	}

	Texture::~Texture() {
		clear();
	}

	bool Texture::IsInitialized() const {
		return m_isInitialized;
	}

	void Texture::clear() {

		if (m_isInitialized) {

			UnbindLocation(m_lastBoundLocation);
			glDeleteTextures(1, &m_ID);
			m_ID = 0;
			m_isInitialized = false;
		}
	}

	void Texture::BindToLocation(const uint8_t activeTextureLocation) {

		MNEMOSY_ASSERT(m_isInitialized, "Trying to bind texture that has no data");

		if (activeTextureLocation > 16 || activeTextureLocation < 0)
		{
			MNEMOSY_ERROR("Can't bind texture to location {} only range 0 to 16 allowed \nBinding to Default location 0", activeTextureLocation);
			glActiveTexture(GL_TEXTURE0);
			m_lastBoundLocation = 0;
		}
		else
		{
		
			glActiveTexture(GL_TEXTURE0 + activeTextureLocation);
			m_lastBoundLocation = activeTextureLocation;
		}
		glBindTexture(GL_TEXTURE_2D, m_ID);
	}

	void Texture::UnbindLocation(const uint8_t activeTextureLocation) {
		
		MNEMOSY_ASSERT(m_isInitialized, "Trying to unbind texture that is not loaded");

		if (activeTextureLocation > 16 || activeTextureLocation < 0) {
			MNEMOSY_CRITICAL("Can't unbind texture from location {} only range 0 to 16 possible", activeTextureLocation);
		}
		else {
			glActiveTexture(GL_TEXTURE0 + activeTextureLocation);
		}

		glBindTexture(GL_TEXTURE_2D, 0);
	}

	unsigned int Texture::GetChannelsAmount(){
		return (unsigned int)TexUtil::get_channels_amount_from_textureFormat(m_textureFormat);
	}

	void Texture::GenerateOpenGlTexture(const PictureInfo& info, const bool generateMipmaps){

		MNEMOSY_ASSERT(Picture::pic_util_check_input_pictureInfo(info).wasSuccessfull,"This method should be provided with a valid picture info");

		// make sure previous texture is cleared if any
		clear();
		
		// update info
		m_width = info.width;
		m_height = info.height;
		m_textureFormat = info.textureFormat;
		m_isHalfFloat = info.isHalfFloat;
		void* pixelBuffer = info.pixels;

		// generate a GL texture
		glGenTextures(1, &m_ID);
		glBindTexture(GL_TEXTURE_2D, m_ID);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);


		uint8_t numChannels, bitsPerChannel, bytesPerPixel;
		graphics::TexUtil::get_information_from_textureFormat(m_textureFormat,numChannels,bitsPerChannel,bytesPerPixel);

		uint32_t rowSize = m_width * bytesPerPixel;

		// ensure byte alignment
		unsigned int unpackAlignment = 4;

		if (rowSize % 4 != 0) {
			unpackAlignment = 1;
		}
		
		glPixelStorei(GL_UNPACK_ALIGNMENT, unpackAlignment);


		
		uint32_t halfFloatType = GL_UNSIGNED_SHORT;
		if (m_isHalfFloat) {
			halfFloatType = GL_HALF_FLOAT;
		}

		switch (m_textureFormat)
		{
			case mnemosy::graphics::MNSY_NONE: break;
			// 8 bit
			case mnemosy::graphics::MNSY_R8:    glTexImage2D(GL_TEXTURE_2D, 0, GL_R8	 , m_width, m_height, 0, GL_RED , GL_UNSIGNED_BYTE, pixelBuffer);	break;
			case mnemosy::graphics::MNSY_RGB8:  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8	 , m_width, m_height, 0, GL_RGB , GL_UNSIGNED_BYTE, pixelBuffer);	break;
			case mnemosy::graphics::MNSY_RGBA8: glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8	 , m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixelBuffer);	break;
			// 16 bit
			case mnemosy::graphics::MNSY_R16:	glTexImage2D(GL_TEXTURE_2D, 0, GL_R16    , m_width, m_height, 0, GL_RED , halfFloatType, pixelBuffer);	break;
			case mnemosy::graphics::MNSY_RGB16:	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16  , m_width, m_height, 0, GL_RGB , halfFloatType, pixelBuffer);	break;
			case mnemosy::graphics::MNSY_RGBA16:glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16 , m_width, m_height, 0, GL_RGBA, halfFloatType, pixelBuffer);	break;
			// 32 bit
			case mnemosy::graphics::MNSY_R32:	glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F	 , m_width, m_height, 0, GL_RED , GL_FLOAT, pixelBuffer);	break;
			case mnemosy::graphics::MNSY_RGB32:	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F , m_width, m_height, 0, GL_RGB , GL_FLOAT, pixelBuffer);	break;
			case mnemosy::graphics::MNSY_RGBA32:glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, m_width, m_height, 0, GL_RGBA, GL_FLOAT, pixelBuffer);	break;
		
			default:break;
		}


		if (generateMipmaps) {
			glGenerateMipmap(GL_TEXTURE_2D);
		}

		m_isInitialized = true;

		return;
	}

} // !mnemosy::graphics