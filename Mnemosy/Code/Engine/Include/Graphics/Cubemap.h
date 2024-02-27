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

		void LoadEquirectangularFromFile(const char* imagePath,const char* name, unsigned int colorCubemapResolution, bool savePermanently);
		
		
		void LoadCubemapsFromKtxFiles(const char* colorCubemapPath, const char* irradianceCubemapPath, const char* prefilterCubemapPath);


		void BindColorCubemap(unsigned int location);
		void BindIrradianceCubemap(unsigned int location);
		void BindPrefilteredCubemap(unsigned int location);


	private:
		void equirectangularToCubemap(unsigned int& cubemapID, unsigned int resolution, bool makeConvolutedIrradianceMap);
		void equirectangularToIrradianceCubemap(unsigned int& cubemapID, unsigned int resolution);
		void equirectangularToPrefilteredCubemap(unsigned int& cubemapID, unsigned int resolution);
		void exportGeneratedCubemapsToKtx(std::string name, unsigned int colorCubemapResolution);


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
