#ifndef IMAGE_BASED_LIGHTING_RENDERER_H
#define IMAGE_BASED_LIGHTING_RENDERER_H

#include "Include/Graphics/Shader.h"

#include <memory>
#include <string>

namespace mnemosy::graphics
{
	struct ModelData;
	//class Shader;
}

namespace mnemosy::graphics
{
	class ImageBasedLightingRenderer 
	{
	public:
		ImageBasedLightingRenderer()  = default;
		~ImageBasedLightingRenderer() = default;

		void Init();
		void Shutdown();
		
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
		// TODO: Replace model loader with mesh registry
		void InitializeMeshAndShader();
		bool IsShaderAndMeshInitialized();


		std::unique_ptr<Shader> m_imagedBasedLightingShader;
		ModelData* m_unitCube = nullptr;
		unsigned int m_brdfLutTextureID = 0;
		unsigned int m_fbo = 0;
		int m_brdfLutResolution = 512;

		bool m_framebufferGenerated = false;
		bool m_brdfLutTexture_isGenerated = false;

		
	};
} // mnemosy::graphics

#endif // !IMAGE_BASED_LIGHTING_RENDERER_H
