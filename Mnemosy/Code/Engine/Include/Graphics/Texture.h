#ifndef TEXTURE_H
#define TEXTURE_H

namespace mnemosy::graphics 
{
	class Texture 
	{
	public:

		Texture();
		~Texture();

		bool generateFromFile(const char* imagePath, bool flipImageVertically, bool generateMipmaps);
		bool containsData() const;
		void clear();
		void BindToLocation(unsigned int activeTextureLocation);
		void UnbindLocation(unsigned int activeTextureLocation);
		// TODO: add functions to set filter mode and Wrap behavior

		int GetChannelsAmount() const;

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
