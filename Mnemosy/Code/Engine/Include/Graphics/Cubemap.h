#ifndef CUBEMAP_H
#define CUBEMAP_H

#include <string>

namespace mnemosy::graphics
{
	class Cubemap
	{
	public:
		Cubemap();
		~Cubemap();

		bool LoadEquirectangularFromFile(const char* imagePath,const char* name, const unsigned int colorCubemapResolution,const bool savePermanently);
		
		void LoadCubemapsFromKtxFiles(const char* colorCubemapPath, const char* irradianceCubemapPath, const char* prefilterCubemapPath);


		void BindColorCubemap(const unsigned int location);
		void BindIrradianceCubemap(const unsigned int location);
		void BindPrefilteredCubemap(const unsigned int location);


	private:
		void equirectangularToCubemap(uint16_t resolution);
		void equirectangularToIrradianceCubemap(uint16_t resolution);
		void equirectangularToPrefilteredCubemap(uint16_t resolution);
		void exportGeneratedCubemapsToKtx(const std::string& name,const unsigned int colorCubemapResolution);


		unsigned int m_equirectangularTextureID = 0;
		unsigned int m_colorCubemapID = 0;
		unsigned int m_irradianceMapID = 0;
		unsigned int m_prefilterMapID = 0;

		// these are global settings and shouldn't be part off each cubemap instance
		const uint16_t m_irradianceMapResolution = 64;
		const uint16_t m_prefilteredMapResolution = 512;


		bool m_equirectangularTexture_isGenerated = false;
		bool m_colorCubemap_isGenerated = false;
		bool m_irradianceMap_isGenerated = false;
		bool m_prefilterMap_isGenerated = false;

		bool m_exportCubemaps = false;
		bool m_loadCubemapsFromFile = false;

	};


} // mnemosy::graphics

#endif // !CUBEMAP_H
