#include "Include/Graphics/Texture.h"
#include "Include/Core/Log.h"

#include "Include/Graphics/TextureDefinitions.h"
#include "Include/Graphics/Image.h"
#include "Include/Graphics/Utils/KtxImage.h"

#include <glad/glad.h>
#include "opencv2/core.hpp"
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/core/utils/logger.hpp>

#include <math.h>

#include <filesystem>
#include "Include/Graphics/Utils/Picture.h"
#include "Include/Core/Utils/StringUtils.h"

namespace mnemosy::graphics
{
	Texture::Texture() {

		cv::utils::logging::setLogLevel(cv::utils::logging::LogLevel::LOG_LEVEL_SILENT);
		//MNEMOSY_INFO("Texture object created")
	}

	Texture::~Texture() {

		clear();
	}

	bool Texture::GenerateFromFile(const char* imagePath,const bool flipImageVertically,const bool generateMipmaps, PBRTextureType pbrType) {
		
		//// test loading tiff files
		//std::filesystem::path pa = std::filesystem::path(imagePath);
		//std::string ext = pa.extension().generic_string();
		//mnemosy::core::StringUtils::MakeStringLowerCase(ext);
		//if (ext == ".tif" || ext == ".tiff") {

		//	return LoadFromFile(imagePath,  flipImageVertically, generateMipmaps,  pbrType);
		//}
		
		clear();
		
		cv::Mat pic = cv::imread(imagePath, cv::IMREAD_UNCHANGED);

		if (pic.empty()) {
			MNEMOSY_ERROR("Texture::GenerateFromFile: OpenCV Image read failed");
			pic.release();
			m_ID = 0;
			m_isInitialized = false;
			return false;					
		}
		

		m_channelsAmount =	pic.channels();
		bool convertSingleChannelToRGB = false;
		if (m_channelsAmount == 1) {
			if (pbrType == graphics::MNSY_TEXTURE_ALBEDO || pbrType == graphics::MNSY_TEXTURE_EMISSION) {
				m_channelsAmount = 3;
				convertSingleChannelToRGB = true;
			}
		}


		m_width = pic.cols;
		m_height = pic.rows;
		int bitDepth = pic.depth();
		cv::flip(pic, pic, 0);
		
		// ensure byte alignment
		int restWidth = m_width % 4;
		
		unsigned int padding = 4;

		if (restWidth != 0) {
			padding = 1;
		}

		glPixelStorei(GL_UNPACK_ALIGNMENT, padding);

		MNEMOSY_TRACE("pixel: {}x{} rest: {}  step: {}, Bit Depth: {}, padding {}",m_width,m_height, restWidth, pic.step1(),bitDepth, padding);



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

			if (convertSingleChannelToRGB) {
				cv::cvtColor(pic, pic, cv::COLOR_GRAY2RGB);
			}
			else {
				cv::cvtColor(pic, pic, cv::COLOR_BGR2RGB);
			}	
			
			
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


			if (bitDepth == 0) {
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, m_width, m_height, 0, GL_RED, GL_UNSIGNED_BYTE, pic.ptr());

			}
			else if (bitDepth == 2) { 

				glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, m_width, m_height, 0, GL_RED, GL_UNSIGNED_SHORT, pic.ptr());
			}
			else if (bitDepth == 4) { // not tested
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, m_width, m_height, 0, GL_RED, GL_FLOAT, pic.ptr());
			}
		}
			
		else if (m_channelsAmount == 2) {
			MNEMOSY_ERROR("Texture: Loading Texures with only 2 or 1 channels is not supported at the moment \nFilepath: {}", imagePath);
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
		MNEMOSY_DEBUG("Loaded Texture from file: {} - {}x{} Channels: {}", imagePath, m_width, m_height, m_channelsAmount);
		return true;
	}

	bool Texture::LoadFromKtx(const char* imagePath) {
		clear();
				
		//glGenTextures(1, &m_ID);
		KtxImage ktxImg;
		bool successfull = ktxImg.LoadKtx(imagePath, m_ID); // should return if successful
		
		m_width = ktxImg.width;
		m_height = ktxImg.height;
		m_channelsAmount = ktxImg.numChannels;

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

	void Texture::LoadIntoCVMat(std::string path) {

		if (path.empty()) {
			MNEMOSY_ERROR("Texture::LoadIntoCVMat: no path set");
			m_matrixLoaded = false;
			return;
		}


		m_cvMat = cv::imread(path.c_str(), cv::IMREAD_UNCHANGED);

		if (m_cvMat.empty()) {

			MNEMOSY_ERROR("Texture::LoadIntoCVMat: OpenCV Image read failed");
			m_cvMat.release();
			m_matrixLoaded = false;
			return;
		}

		cv::flip(m_cvMat, m_cvMat, 0);

		m_matrixLoaded = true;
		return;
	}

	bool Texture::GenerateFromCVMat() {

		if (m_cvMat.empty()) {
			MNEMOSY_ERROR("Texture::generateFromCVMat:: Mat is empty");
			m_ID = 0;
			m_isInitialized = false;
			return false;
		}

		m_channelsAmount = m_cvMat.channels();
		m_width = m_cvMat.cols;
		m_height = m_cvMat.rows;
		int bitDepth = m_cvMat.depth();

		// ensure byte alignment
		int restWidth = m_width % 4;

		unsigned int padding = 4;

		if (restWidth != 0) {
			padding = 1;
		}

		glPixelStorei(GL_UNPACK_ALIGNMENT, padding);


		glGenTextures(1, &m_ID);
		glBindTexture(GL_TEXTURE_2D, m_ID);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		//depth 0 = 8bit unsigned byte, 2 0 16bit UNSIGNED SHORT // 4 not tested

		//Loading with RGBA channels
		if (m_channelsAmount == 4) {
			cv::cvtColor(m_cvMat, m_cvMat, cv::COLOR_BGRA2RGBA);
			// cv::pow(pic, 2.2f,pic); if we do gamma corrrect here we need to only apply it to color textures 
			// cv::pow requres input to be in 32 or 64 bit...

			if (bitDepth == 0) {
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_cvMat.ptr());
			}
			else if (bitDepth == 2) {

				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_SHORT, m_cvMat.ptr());
			}
			else if (bitDepth == 4) { // not tested

				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_width, m_height, 0, GL_RGBA, GL_FLOAT, m_cvMat.ptr());
			}
		}
		// load with RGB channels
		else if (m_channelsAmount == 3) {

			cv::cvtColor(m_cvMat,m_cvMat, cv::COLOR_BGR2RGB);


			if (bitDepth == 0) {
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_width, m_height, 0, GL_RGB, GL_UNSIGNED_BYTE, m_cvMat.ptr());
			}
			else if (bitDepth == 2) {
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_width, m_height, 0, GL_RGB, GL_UNSIGNED_SHORT, m_cvMat.ptr());
			}
			else if (bitDepth == 4) { // not tested
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_width, m_height, 0, GL_RGB, GL_FLOAT, m_cvMat.ptr());
			}
		}

		else if (m_channelsAmount == 1) {

			if (bitDepth == 0) {
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, m_width, m_height, 0, GL_RED, GL_UNSIGNED_BYTE, m_cvMat.ptr());

			}
			else if (bitDepth == 2) { // not tested

				glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, m_width, m_height, 0, GL_RED, GL_UNSIGNED_SHORT, m_cvMat.ptr());
			}
			else if (bitDepth == 4) { // not tested
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, m_width, m_height, 0, GL_RED, GL_FLOAT, m_cvMat.ptr());
			}
		}

		else if (m_channelsAmount == 2) {
			m_isInitialized = false;
			glDeleteTextures(1, &m_ID);
			m_cvMat.release();
			return false;
		}

		m_cvMat.release();

		glGenerateMipmap(GL_TEXTURE_2D);
		
		m_isInitialized = true;
		MNEMOSY_TRACE("Loaded Texture (from Matrix): {}x{}", m_width, m_height);
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

		if (m_matrixLoaded) {

			if (!m_cvMat.empty()) {
				m_cvMat.release();
			}

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

	// in development not fully features yet
	bool Texture::LoadFromFile(const char* imagePath, const bool flipImageVertically, const bool generateMipmaps, PBRTextureType pbrType) {


		MNEMOSY_WARN("Loading with libtiff");

		graphics::PictureErrorCode errorCode = PictureErrorCode::MNSY_PICTURE_ERROR_NONE;
		
		graphics::PictureInfo info = graphics::Picture::ReadTiff(errorCode, imagePath, true);

		if (errorCode != graphics::PictureErrorCode::MNSY_PICTURE_ERROR_NONE) {
			MNEMOSY_ERROR("Texture::LoadFromFile: Failed to load tiff image: {} \nMessage: {}", imagePath, graphics::Picture::ErrorStringGet(errorCode));

			if (info.pixels != nullptr) {
				free(info.pixels);
			}

			return false;
		}


		graphics::TextureFormat format = info.textureFormat;


		uint8_t channels = 0;
		uint8_t bitsPerChannel = 0; // will be either 8 , 16  or 32
		size_t bytesPerPixel = 0;

		graphics::TextureDefinitions::GetInfoFromTextureFormat(format,channels,bitsPerChannel,bytesPerPixel);
		m_channelsAmount = channels;
		m_width = info.width;
		m_height = info.height;



		bool convertSingleChannelToRGB = false;
		if (m_channelsAmount == 1) {
			if (pbrType == graphics::MNSY_TEXTURE_ALBEDO || pbrType == graphics::MNSY_TEXTURE_EMISSION) {
				m_channelsAmount = 3;
				convertSingleChannelToRGB = true;
			}
		}

		//depth 0 = 8bit unsigned byte, 2 0 16bit UNSIGNED SHORT // 4 not tested

		//int bitDepth = pic.depth();
		//cv::flip(pic, pic, 0);

		// ensure byte alignment
		int restWidth = m_width % 4;
		unsigned int padding = 4;

		if (restWidth != 0) {
			padding = 1;
		}

		glPixelStorei(GL_UNPACK_ALIGNMENT, padding);

		MNEMOSY_TRACE("pixel: {}x{} format: {}", m_width, m_height, graphics::TextureDefinitions::GetTextureFormatAsString(format));


		glGenTextures(1, &m_ID);
		glBindTexture(GL_TEXTURE_2D, m_ID);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		//depth 0 = 8bit unsigned byte, 2 0 16bit UNSIGNED SHORT // 4 not tested


		if (format == MNSY_RG8 || format == MNSY_RG16 || format == MNSY_RG32) {
			// error not supported
			MNEMOSY_WARN("Faild to load image: loading textures with 2 channels is not supported. {}",imagePath);
			m_ID = 0;
			m_isInitialized = false;
			return false;
		}

		// TODO: convert to rgb if convertSingleChannelToRGB is true


		// upload the gl texture
		
		void* pixelBuffer = info.pixels;

		switch (format)
		{
		case mnemosy::graphics::MNSY_NONE: break;
		// 8 bit
		case mnemosy::graphics::MNSY_R8:    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED , m_width, m_height, 0, GL_RED , GL_UNSIGNED_BYTE, pixelBuffer);	break;
		case mnemosy::graphics::MNSY_RGB8:  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB , m_width, m_height, 0, GL_RGB , GL_UNSIGNED_BYTE, pixelBuffer);	break;
		case mnemosy::graphics::MNSY_RGBA8: glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixelBuffer);	break;
		// 16 bit
		case mnemosy::graphics::MNSY_R16:	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED , m_width, m_height, 0, GL_RED , GL_UNSIGNED_SHORT, pixelBuffer);	break;
		case mnemosy::graphics::MNSY_RGB16:	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB , m_width, m_height, 0, GL_RGB , GL_UNSIGNED_SHORT, pixelBuffer);	break;
		case mnemosy::graphics::MNSY_RGBA16:glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_SHORT, pixelBuffer);	break;
		// 32 bit
		case mnemosy::graphics::MNSY_R32:	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED , m_width, m_height, 0, GL_RED , GL_FLOAT, pixelBuffer);			break;
		case mnemosy::graphics::MNSY_RGB32:	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB , m_width, m_height, 0, GL_RGB , GL_FLOAT, pixelBuffer);			break;
		case mnemosy::graphics::MNSY_RGBA32:glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_width, m_height, 0, GL_RGBA, GL_FLOAT, pixelBuffer);			break;
		
		default:break;
		}


		m_isInitialized = true;
		free(pixelBuffer);

		if (generateMipmaps) {
			glGenerateMipmap(GL_TEXTURE_2D);
		}

		return true;
	}

} // !mnemosy::graphics