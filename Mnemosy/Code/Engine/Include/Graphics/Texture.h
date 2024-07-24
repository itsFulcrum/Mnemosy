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
		bool generateFromFile(const char* imagePath,const bool flipImageVertically,const bool generateMipmaps);
		bool LoadFromKtx(const char* imagePath);

		void LoadIntoCVMat();
		bool generateFromCVMat();



		bool containsData() const;
		void clear();
		void BindToLocation(const unsigned int activeTextureLocation);
		void UnbindLocation(const unsigned int activeTextureLocation);
		// TODO: add functions to set filter mode and Wrap behavior

		unsigned int GetID() { return m_ID; }
		unsigned int GetChannelsAmount() { return m_channelsAmount; };
		unsigned int GetWidth() { return m_width; }
		unsigned int GetHeight() { return m_height; }

		std::string m_path;
		bool matrixLoaded = false;
	private:
		bool m_isInitialized = false;
		unsigned int m_ID = 0;
		int m_lastBoundLocation = 0;
		
		unsigned int m_channelsAmount = 0;
		unsigned int m_width = 0;
		unsigned int m_height = 0;

		
		cv::Mat m_cvMat;
	};

} // mnemosy::graphics

#endif //!TEXTURE_H
