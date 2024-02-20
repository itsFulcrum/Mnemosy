#include "Engine/Include/Graphics/Texture.h"
#include "Engine/Include/Core/Log.h"
#include "Engine/Include/Graphics/Image.h"

#include <glad/glad.h>

namespace mnemosy::graphics
{
	Texture::Texture() 
	{
	
	}

	Texture::~Texture()
	{
		clear();
	}

	void Texture::generateFromFile(const char* imagePath, bool flipImageVertically, bool generateMipmaps)
	{
		clear();

		Image* img = new Image();

		bool successfull = img->LoadImageFromFile(imagePath, flipImageVertically);

		//int channels, width, height;
		//const char* textureData = stbi_load(imagePath, &width, &height, &channels, 0);


		if (successfull)
		{
			m_channelsAmount =	img->channels;
			m_width = img->width;
			m_height = img->height;

			glGenTextures(1, &m_ID);
			glBindTexture(GL_TEXTURE_2D, m_ID);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			if (m_channelsAmount == 4)
			{
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, img->width, img->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, img->imageData);
			}
			else if (m_channelsAmount == 3)
			{
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, img->width, img->height, 0, GL_RGB, GL_UNSIGNED_BYTE, img->imageData);
			}
			else if (m_channelsAmount == 2)
			{
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16, img->width, img->height, 0, GL_RG, GL_UNSIGNED_BYTE, img->imageData);

			}
			else if (m_channelsAmount == 1)
			{
				MNEMOSY_ERROR("Texures with only 1 channel are not supported \nFilepath: {}", imagePath);
				m_ID = 0;
				m_isInitialized = false;
				//img->FreeData();
				delete img;
				img = nullptr;
				return;

			}

			if (generateMipmaps)
			{
				glGenerateMipmap(GL_TEXTURE_2D);
			}

			m_isInitialized = true;
			MNEMOSY_INFO("Loaded Texture from file: {}", imagePath);


		}
		else
		{
			MNEMOSY_ERROR("Failed to load texture \nFilepath: {}", imagePath);
			m_ID = 0;
			m_isInitialized = false;
		}


		//img->FreeData();
		delete img;
		img = nullptr;

	}


	bool Texture::containsData() const
	{
		return m_isInitialized;
	}

	void Texture::clear()
	{
		if (m_isInitialized)
		{
			UnbindLocation(m_lastBoundLocation);
			glDeleteTextures(1, &m_ID);
			m_ID = 0;
			m_isInitialized = false;
		}
	}

	void Texture::BindToLocation(unsigned int activeTextureLocation)
	{
		MNEMOSY_ASSERT(m_isInitialized, "Trying to bind texture that has no data");
		if (!m_isInitialized) 
			return;


		if (activeTextureLocation > 16 || activeTextureLocation < 0)
		{
			MNEMOSY_ERROR("Can't bind texture to location {} only range 0 to 16 allowed \nBinding to Default location 0", activeTextureLocation)
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

	void Texture::UnbindLocation(unsigned int activeTextureLocation)
	{

		if (activeTextureLocation > 16 || activeTextureLocation < 0)
		{
			MNEMOSY_CRITICAL("Can't unbind texture from location {} only range 0 to 16 possible", activeTextureLocation)
		}
		else
		{
			glActiveTexture(GL_TEXTURE0 + activeTextureLocation);
		}
		glBindTexture(GL_TEXTURE_2D, m_ID);

	}

	int Texture::GetChannelsAmount() const
	{
		if (!m_isInitialized) {
			MNEMOSY_ERROR("Trying to read data from an uninitialize texture")
			return 0;
		}

		return m_channelsAmount;
	}

} // end namespace mnemosy::graphics