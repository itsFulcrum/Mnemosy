#ifndef IMAGE_BASED_LIGHTING_RENDERER_H
#define IMAGE_BASED_LIGHTING_RENDERER_H


namespace mnemosy::graphics
{
	struct ModelData;
	class Shader;
}

#include <memory>

namespace mnemosy::graphics
{
	class ImageBasedLightingRenderer 
	{
	public:
		ImageBasedLightingRenderer();
		~ImageBasedLightingRenderer();
		
		void RenderEquirectangularToCubemapTexture(unsigned int& cubemapTextureID, unsigned int& equirectangularTextureID, unsigned int textureRes, bool makeConvolutedIrradianceMap);
		void RenderEquirectangularToPrefilteredCubemapTexture(unsigned int& cubemapID, unsigned int& equirectangularTextureID, unsigned int resolution);
		void RenderBrdfLutTexture();

		void BindBrdfLutTexture(unsigned int location);

	private:
		void DrawIntoFramebuffer();

	private:
		unsigned int m_fbo = 0;

		ModelData* m_unitCube = nullptr;
		std::unique_ptr<Shader> m_imagedBasedLightingShader;
		unsigned int m_brdfLutTextureID = 0;
		bool m_brdfLutTexture_isGenerated = false;

		int m_brdfLutResolution = 512;
		

	};
}

#endif // !IMAGE_BASED_LIGHTING_RENDERER_H