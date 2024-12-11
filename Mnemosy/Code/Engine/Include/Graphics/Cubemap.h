#ifndef CUBEMAP_H
#define CUBEMAP_H

#include <string>
#include <filesystem>

namespace mnemosy::graphics {
	class Texture;
}


namespace mnemosy::graphics
{
	enum CubemapType {
		MNSY_CUBEMAP_TYPE_COLOR = 0,
		MNSY_CUBEMAP_TYPE_IRRADIANCE = 1,
		MNSY_CUBEMAP_TYPE_PREFILTER = 2,
		MNSY_CUBEMAP_TYPE_COUNT,
		MNSY_CUBEMAP_TYPE_NONE
	};

	class Cubemap
	{
	public:
		Cubemap() = default;
		~Cubemap();

		// if overrideTexResolution is set skyboxes of type Color will use specified resolution otherwise they will use height of the equirectangular texture. other types are not effectd
		void GenerateOpenGlCubemap_FromEquirecangularTexture(Texture& equirectangularTex, CubemapType cubeType, bool overrideTexResolution, unsigned int _resolution);


		void GenerateOpenGlCubemap_FromKtx2File(std::filesystem::path& path, bool loadStoredMips);

		void Clear();

		void Bind(const unsigned int location);
		void Unbind();

		unsigned int& GetGlID() { return m_gl_ID; }
		uint16_t GetResolution() { return m_resolution; }

	private:
		unsigned int m_gl_ID = 0;
		bool m_isInitialized = false;

		uint16_t m_resolution = 0;
		uint8_t m_lastBoundLocation = 0;
	};


} // mnemosy::graphics

#endif // !CUBEMAP_H
