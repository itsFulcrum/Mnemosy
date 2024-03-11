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
		void equirectangularToCubemap(unsigned int resolution);
		void equirectangularToIrradianceCubemap(unsigned int resolution);
		void equirectangularToPrefilteredCubemap(unsigned int resolution);
		void exportGeneratedCubemapsToKtx(const std::string& name,const unsigned int colorCubemapResolution);


		unsigned int m_equirectangularTextureID = 0;
		bool m_equirectangularTexture_isGenerated = false;

		unsigned int m_colorCubemapID = 0;
		bool m_colorCubemap_isGenerated = false;

		unsigned int m_irradianceMapID = 0;
		bool m_irradianceMap_isGenerated = false;

		unsigned int m_prefilterMapID = 0;
		bool m_prefilterMap_isGenerated = false;

		unsigned int m_irradianceMapResolution = 64;
		unsigned int m_prefilteredMapResolution = 512;


		bool m_exportCubemaps = false;
		bool m_loadCubemapsFromFile = false;

	};


} // mnemosy::graphics

#endif // !CUBEMAP_H
