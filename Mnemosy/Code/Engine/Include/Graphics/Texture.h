#ifndef TEXTURE_H
#define TEXTURE_H


#include <string>

//#include "opencv2/core/mat.hpp"


namespace mnemosy::graphics {
	enum PBRTextureType;
	enum TextureFormat;
	struct PictureInfo;
}


namespace mnemosy::graphics 
{
	class Texture {
	public:

		Texture();
		~Texture();
		
		void GenerateOpenGlTexture(const PictureInfo& info, const bool generateMipmaps);


		bool IsInitialized() const;
		void clear();
		void BindToLocation(const uint8_t activeTextureLocation);
		void UnbindLocation(const uint8_t activeTextureLocation);
		
		unsigned int& GetID() { return m_ID; }
		unsigned int GetChannelsAmount();


		TextureFormat GetTextureFormat() {return m_textureFormat;}
		bool IsHalfFloat() { return m_isHalfFloat; }
		unsigned int GetWidth() { return m_width; }
		unsigned int GetHeight() { return m_height; }

	private:
		unsigned int m_ID = 0;
		TextureFormat m_textureFormat;

		uint16_t m_width = 0; 
		uint16_t m_height = 0; 

		bool m_isInitialized = false; 
		uint8_t m_lastBoundLocation = 0; 
		bool m_isHalfFloat = false; 
	};

} // mnemosy::graphics

#endif //!TEXTURE_H
