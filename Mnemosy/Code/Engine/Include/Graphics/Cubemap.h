#ifndef GRAPHICS_CUBEMAP_H
#define GRAPHICS_CUBEMAP_H


namespace mnemosy::graphics
{

	class Cubemap
	{
	public:

		Cubemap();
		~Cubemap();

		void LoadEquirectangularFromFile(const char* path, unsigned int resolution, bool generateConvolutionMap);

		void BindColorCubemap(unsigned int location);
		void BindIrradianceCubemap(unsigned int location);
		void BindPrefilteredCubemap(unsigned int location);


	private:
		void equirectangularToCubemap(unsigned int& cubemapID, unsigned int resolution, bool makeConvolutedIrradianceMap);
		void equirectangularToPrefilteredCubemap(unsigned int& cubemapID, unsigned int resolution);

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
		bool m_loadCubemapsFromFile = true;

	};


}



#endif // !GRAPHICS_CUBEMAP_H
