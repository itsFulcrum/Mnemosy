#ifndef TEXTURE_H
#define TEXTURE_H

#include <iostream>


#include<stb_image.h>


class Texture 
{
public:
	unsigned int ID = NULL;

private:
	bool m_hasAlphaChannel = false;
	int m_channelsAmount = 0;
	int m_lastBoundLocation = 0;

public:

	Texture()
	{
	};
	~Texture()
	{
		glDeleteTextures(1,&ID);
	};

	void generateFromFile(const char* imagePath, bool flipImageVertically, bool generateMipmaps)
	{
		clear();

		glGenTextures(1, &ID);
		glBindTexture(GL_TEXTURE_2D, ID);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		//load image using stb_image.h
		int width, height, channelsAmount;

		stbi_set_flip_vertically_on_load(flipImageVertically);
		//stbi_ldr_to_hdr_gamma(1.0f/2.2f);


		unsigned char* textureData = stbi_load(imagePath, &width, &height, &channelsAmount, 0);


		m_channelsAmount = channelsAmount;
		if (textureData)
		{
			if (channelsAmount == 3)
			{
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, textureData);
				m_hasAlphaChannel = false;
			}
			else if (channelsAmount == 4)
			{
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, textureData);
				m_hasAlphaChannel = true;
			}
			else if (channelsAmount > 4)
			{
				std::cout << "ERROR::TEXTURE::HAS_MORE_THAN_4_CHANNELS\n" << "Filepath: " << imagePath << "\n" << "-------------------------------" << std::endl;
				glDeleteTextures(1, &ID);
				ID = NULL;
			}
			else
			{
				std::cout << "ERROR::TEXTURE::HAS_LESS_THAN_3_CHANNELS\n" << "Filepath: " << imagePath << "\n" << "-------------------------------" << std::endl;
				glDeleteTextures(1, &ID);
				ID = NULL;
			}

			if (generateMipmaps)
			{
				glGenerateMipmap(GL_TEXTURE_2D);
			}
		}
		else
		{
			std::cout << "ERROR::TEXTURE::FAILD_TO_LOAD\n" << "Filepath: " << imagePath << "\n" << "\n -- --------------------------------------------------- -- " << std::endl;
			glDeleteTextures(1, &ID);
			ID = NULL;
		}

		stbi_image_free(textureData);
	}


	bool containsData()
	{
		if (ID != NULL)
			return true;

		return false;
	}

	void clear()
	{
		if (containsData())
		{
			UnbindLocation(m_lastBoundLocation);
			glDeleteTextures(1, &ID);
			ID = NULL;
		}
	}

	void BindToLocation(unsigned int activeTextureLocation)
	{
		if (ID == NULL) {
			std::cout << "ERROR::TEXTURE::NO_TEXTURE_HAS_BEEN_GENEREATED" << std::endl;
			return;
		}

		if (activeTextureLocation > 16 || activeTextureLocation < 0) 
		{
			std::cout << "ERROR::TEXTURE::CANT_BIND_TEXTURE_TO_LOCATION: " << activeTextureLocation << " ONLY_RANGE_0_TO_16_ALLOWED\n" << "BOUND_TO_DEFAULT_LOCATION: 0\n" << "\n -- --------------------------------------------------- -- " << std::endl;
			glActiveTexture(GL_TEXTURE0);
			m_lastBoundLocation = 0;
		}
		else
		{
			glActiveTexture(GL_TEXTURE0 + activeTextureLocation);
			m_lastBoundLocation = activeTextureLocation;
		}
		glBindTexture(GL_TEXTURE_2D, ID);

	}
	

	void UnbindLocation(unsigned int activeTextureLocation)
	{
		
		if (activeTextureLocation > 16 || activeTextureLocation < 0)
		{
			std::cout << "ERROR::TEXTURE::CANT_UNBIND_TEXTURE_LOCATION: " << activeTextureLocation << " ONLY_RANGE_0_TO_16_ALLOWED\n"  << " -- --------------------------------------------------- -- " << std::endl;
			//glActiveTexture(GL_TEXTURE0);
		}
		else
		{
			glActiveTexture(GL_TEXTURE0 + activeTextureLocation);
		}
		glBindTexture(GL_TEXTURE_2D, ID);
		
	}


	bool HasAlphaCannel()
	{
		if (ID == NULL)
			return NULL;


		return m_hasAlphaChannel;
	}
	int GetChannelsAmount() 
	{
		if (ID == NULL)
			return NULL;

		return m_channelsAmount;
	}

private:
	void PrintConsolMessage(std::string message)
	{
		std::cout << message << std::endl;
	}


	
	
};


#endif 
