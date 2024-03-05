#ifndef TEXTURE_H
#define TEXTURE_H

namespace mnemosy::graphics 
{
	class Texture 
	{
	public:

		Texture();
		~Texture();

		const bool generateFromFile(const char* imagePath,const bool flipImageVertically,const bool generateMipmaps);
		const bool containsData() const;
		void clear();
		void BindToLocation(const unsigned int activeTextureLocation);
		void UnbindLocation(const unsigned int activeTextureLocation);
		// TODO: add functions to set filter mode and Wrap behavior

		const int GetChannelsAmount() const;

	private:
		unsigned int m_ID = 0;
		bool m_isInitialized = false;
		int m_channelsAmount = 0;
		int m_lastBoundLocation = 0;
		unsigned int m_width = 0;
		unsigned int m_height = 0;

	};

} // mnemosy::graphics

#endif //!TEXTURE_H
