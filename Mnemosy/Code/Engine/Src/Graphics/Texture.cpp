#include "Include/Graphics/Texture.h"
#include "Include/Core/Log.h"

#include "Include/Core/Log.h"
#include "Include/Graphics/Image.h"
#include "Include/Graphics/Utils/KtxImage.h"

#include <glad/glad.h>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/core/utils/logger.hpp>

#include <math.h>

namespace mnemosy::graphics
{
	Texture::Texture() {

		cv::utils::logging::setLogLevel(cv::utils::logging::LogLevel::LOG_LEVEL_SILENT);
		//MNEMOSY_INFO("Texture object created")
	}

	Texture::~Texture() {

		clear();
	}

	bool Texture::generateFromFile(const char* imagePath,const bool flipImageVertically,const bool generateMipmaps) {
		clear();
		
		cv::Mat pic = cv::imread(imagePath, cv::IMREAD_UNCHANGED);

		if (pic.empty()) {
			MNEMOSY_ERROR("Texture::generateFromFile: OpenCV Image read failed");
			pic.release();
			//m_ID = 0;
			m_isInitialized = false;
			return false;					
		}
		
		m_channelsAmount =	pic.channels();
		m_width = pic.cols;
		m_height = pic.rows;
		int bitDepth = pic.depth();
		cv::flip(pic, pic, 0);
			
		MNEMOSY_DEBUG("Texture::gnerateFromFile: Loading image: {}\nWidht: {} Height: {} Channels: {} Depth: {}", imagePath, m_width, m_height, m_channelsAmount, pic.depth());
					
		glGenTextures(1, &m_ID);
		glBindTexture(GL_TEXTURE_2D, m_ID);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		
		//depth 0 = 8bit unsigned byte, 2 0 16bit UNSIGNED SHORT // 4 not tested

		//Loading with RGBA channels
		if (m_channelsAmount == 4) {
			cv::cvtColor(pic, pic, cv::COLOR_BGRA2RGBA);
			// cv::pow(pic, 2.2f,pic); if we do gamma corrrect here we need to only apply it to color textures 
			// cv::pow requres input to be in 32 or 64 bit...

			if (bitDepth == 0) {
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pic.ptr());
			}
			else if (bitDepth == 2) {

				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_SHORT, pic.ptr());
			}
			else if (bitDepth == 4) { // not tested

				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_width, m_height, 0, GL_RGBA, GL_FLOAT, pic.ptr());
			}
		}
		// load with RGB channels
		else if (m_channelsAmount == 3) {

			cv::cvtColor(pic, pic, cv::COLOR_BGR2RGB);
			// cv::pow(pic, 2.2f,pic); if we do gamma corrrect here we need to only apply it to color textures 
			// cv::pow requres input to be in 32 or 64 bit...
			
			
			if (bitDepth == 0) {
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_width, m_height, 0, GL_RGB, GL_UNSIGNED_BYTE, pic.ptr());
			}
			else if (bitDepth == 2) {
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_width, m_height, 0, GL_RGB, GL_UNSIGNED_SHORT, pic.ptr());
			}
			else if (bitDepth == 4) { // not tested
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_width, m_height, 0, GL_RGB, GL_FLOAT, pic.ptr());
			}
		}

		else if (m_channelsAmount == 1) {

			//cv::cvtColor(pic, pic, cv::COLOR_BGR2RGB);
			
			if (bitDepth == 0) {
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_width, m_height, 0, GL_RED, GL_UNSIGNED_BYTE, pic.ptr());

			}
			else if (bitDepth == 2) { // not tested

				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_width, m_height, 0, GL_RGB, GL_UNSIGNED_SHORT, pic.ptr());
			}
			else if (bitDepth == 4) { // not tested
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_width, m_height, 0, GL_RGB, GL_FLOAT, pic.ptr());
			}
		}
			
		else if (m_channelsAmount == 2) {
			MNEMOSY_ERROR("Texture::generateFromFile: Loading Texures with only 2 or 1 channels is not supported at the moment \nFilepath: {}", imagePath);
			//m_ID = 0;
			m_isInitialized = false;
			glDeleteTextures(1,&m_ID);
			pic.release();
			return false;
		}
		pic.release();

		if (generateMipmaps) {
			glGenerateMipmap(GL_TEXTURE_2D);
		}
		m_isInitialized = true;

		return true;
	}

	bool Texture::LoadFromKtx(const char* imagePath) {
		clear();
				
		//glGenTextures(1, &m_ID);
		KtxImage ktxImg;
		bool successfull = ktxImg.LoadKtx(imagePath, m_ID); // should return if successful
		
		if (!successfull) {
			glDeleteTextures(1, &m_ID);
			m_ID = 0;
			m_isInitialized = false;
			return false;
		}
		//MNEMOSY_DEBUG("Gen GL Texture ID: {}",m_ID);


		glBindTexture(GL_TEXTURE_2D, m_ID);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);


		// lets try if its loading them from the file
		// glGenerateMipmap(GL_TEXTURE_2D);
		
		m_isInitialized = true;

		return true;
	}


	bool Texture::containsData() const {
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

	void Texture::BindToLocation(const unsigned int activeTextureLocation) {

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

	void Texture::UnbindLocation(const unsigned int activeTextureLocation) {

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

} // !mnemosy::graphics