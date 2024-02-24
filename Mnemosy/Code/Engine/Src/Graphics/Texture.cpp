#include "Engine/Include/Graphics/Texture.h"
#include "Engine/Include/Core/Log.h"
#include "Engine/Include/Graphics/Image.h"

#include <glad/glad.h>

#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

#include "Engine/Include/Graphics/Utils/KtxImage.h"

namespace mnemosy::graphics
{
	Texture::Texture() 
	{
		
	}

	Texture::~Texture()
	{
		clear();
	}

	bool Texture::generateFromFile(const char* imagePath, bool flipImageVertically, bool generateMipmaps)
	{
		clear();

		/*Image* img = new Image();

		bool successfull = img->LoadImageFromFile(imagePath, flipImageVertically);

		if (successfull)
		{
			MNEMOSY_TRACE("StbLoaded img  numChannels: {}", img->channels);
			KtxImage ktxImg;
			ktxImg.SaveKtx("../Resources/Textures/Tom5_linear.ktx2", img->imageData, img->width);
		}*/

		//successfull = false;
		//int channels, width, height;
		//const char* textureData = stbi_load(imagePath, &width, &height, &channels, 0);
		

		// TEST Loading Ktx Image
		bool loadKTX = false;
		if (loadKTX)
		{
			glGenTextures(1, &m_ID);
			KtxImage ktxImg;
			ktxImg.LoadKtx(imagePath, m_ID); // should return if successful
			//ktxImg.SaveKtx(imagePath, 1024);

			

			glBindTexture(GL_TEXTURE_2D, m_ID);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			if (generateMipmaps)
			{
				glGenerateMipmap(GL_TEXTURE_2D);
			}

			m_isInitialized = true;

			return true;
		}
		// TEST Loading Ktx Image end

		cv::Mat pic = cv::imread(imagePath, cv::IMREAD_COLOR);
		



		if (pic.empty())
		{
			MNEMOSY_ERROR("opencv image read failed");

			pic.release();
			
			
			m_ID = 0;
			m_isInitialized = false;
			//img->FreeData();
			//delete img;
			//img = nullptr;
			return false;
		}
		

		if (!pic.empty())
		{

			// save img as ktx2 file
			bool saveKtx = true;
			if(saveKtx)
			{
				cv::Mat rgba;
				cv::cvtColor(pic, rgba, cv::COLOR_BGR2RGB); // convert to rgb
			
				MNEMOSY_TRACE("openCV loaded img  numChannels: {}", rgba.channels());
				KtxImage ktxImg;
				bool worked = ktxImg.SaveKtx("../Resources/Textures/ExportTest2_linear.ktx2", rgba.ptr(),rgba.channels(), rgba.cols, rgba.rows);
				if(worked)
					MNEMOSY_TRACE("Exported ktx");
				rgba.release();
			}



			m_channelsAmount =	pic.channels();
			m_width = pic.cols;
			m_height = pic.rows;

			MNEMOSY_DEBUG("Loading image with openCV: widht: {} Height: {} channels: {} ", m_width, m_height, m_channelsAmount);

			cv::flip(pic, pic, 0);

			glGenTextures(1, &m_ID);
			glBindTexture(GL_TEXTURE_2D, m_ID);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			if (m_channelsAmount == 4)
			{
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_width, m_height, 0, GL_BGRA, GL_UNSIGNED_BYTE, pic.ptr());
			}
			else if (m_channelsAmount == 3)
			{
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_width, m_height, 0, GL_BGR, GL_UNSIGNED_BYTE, pic.ptr());
			}
			else if (m_channelsAmount == 2)
			{
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16, m_width, m_height, 0, GL_RG, GL_UNSIGNED_BYTE, pic.ptr());

			}
			else if (m_channelsAmount == 1)
			{
				MNEMOSY_ERROR("Texures with only 1 channel are not supported \nFilepath: {}", imagePath);
				m_ID = 0;
				m_isInitialized = false;
				pic.release();


				return false;

			}

			if (generateMipmaps)
			{
				glGenerateMipmap(GL_TEXTURE_2D);
			}

			m_isInitialized = true;
			MNEMOSY_INFO("Loaded Texture from file: {}", imagePath);

			//delete img;
			//img = nullptr;
			pic.release();
			return true;
		}


		pic.release();
		return false;

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