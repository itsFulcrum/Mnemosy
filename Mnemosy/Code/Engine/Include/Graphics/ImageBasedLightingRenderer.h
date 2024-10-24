#ifndef IMAGE_BASED_LIGHTING_RENDERER_H
#define IMAGE_BASED_LIGHTING_RENDERER_H

#include <memory>
#include <string>

namespace mnemosy::graphics
{
	struct ModelData;
	class Shader;
}

namespace mnemosy::graphics
{
	class ImageBasedLightingRenderer 
	{
	public:
		ImageBasedLightingRenderer();
		~ImageBasedLightingRenderer();
		
		void RenderEquirectangularToCubemapTexture(unsigned int& cubemapTextureID, unsigned int& equirectangularTextureID, unsigned int textureRes);
		void RenderEquirectangularToIrradianceCubemapTexture(unsigned int& cubemapTextureID, unsigned int& equirectangularTextureID, unsigned int textureRes);
		void RenderEquirectangularToPrefilteredCubemapTexture(unsigned int& cubemapID, unsigned int& equirectangularTextureID, unsigned int resolution);
		void PrepareCubemapRendering();
		void EndCubemapRendering();

		void BindBrdfLutTexture(unsigned int location);

	private:
		void RenderBrdfLutTextureAndSafeKtx(const char* exportpath,bool exportToFile);
		void LoadBrdfLutTexture();

		void DrawIntoFramebuffer();

	private:
		void InitializeMeshAndShader();
		bool IsShaderAndMeshInitialized();

		bool m_framebufferGenerated = false;
		unsigned int m_fbo = 0;

		ModelData* m_unitCube = nullptr;
		std::unique_ptr<Shader> m_imagedBasedLightingShader;
		unsigned int m_brdfLutTextureID = 0;
		bool m_brdfLutTexture_isGenerated = false;

		int m_brdfLutResolution = 512;
		
	};
} // mnemosy::graphics

#endif // !IMAGE_BASED_LIGHTING_RENDERER_H
