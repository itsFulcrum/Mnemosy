#ifndef TEXTURE_H
#define TEXTURE_H


#include <string>

#include "opencv2/core/mat.hpp"

namespace mnemosy::graphics 
{
	class Texture {
	public:

		Texture();
		~Texture();
		// todo remove flip parameter
		bool GenerateFromFile(const char* imagePath,const bool flipImageVertically,const bool generateMipmaps);
		bool LoadFromKtx(const char* imagePath);

		void LoadIntoCVMat(std::string path);
		bool GenerateFromCVMat();



		bool containsData() const;
		void clear();
		void BindToLocation(const unsigned int activeTextureLocation);
		void UnbindLocation(const unsigned int activeTextureLocation);
		// TODO: add functions to set filter mode and Wrap behavior

		unsigned int GetID() { return m_ID; }
		unsigned int GetChannelsAmount() { return m_channelsAmount; };
		unsigned int GetWidth() { return m_width; }
		unsigned int GetHeight() { return m_height; }

	private:
		cv::Mat m_cvMat;
		bool m_matrixLoaded = false;

		bool m_isInitialized = false;

		unsigned int m_ID = 0;
		unsigned int m_lastBoundLocation = 0;
		
		unsigned int m_channelsAmount = 0;
		unsigned int m_width = 0;
		unsigned int m_height = 0;


		
	};

} // mnemosy::graphics

#endif //!TEXTURE_H
