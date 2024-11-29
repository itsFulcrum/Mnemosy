#ifndef GRAPHICS_RENDERER_H
#define GRAPHICS_RENDERER_H
#include "Include/MnemosyConfig.h"

#include "Include/Core/Utils/FileWatcher.h"
#include <glm/glm.hpp>

#include <filesystem>

namespace mnemosy::systems {
	enum LibEntryType;
}

namespace mnemosy::graphics
{
	class Shader;
	class RenderMesh;
	class Skybox;
	class UnlitMaterial;
	class PbrMaterial;
	class Light;
	class Scene;
}

namespace mnemosy::graphics
{

	enum MSAAsamples
	{
		MSAAOFF,
		MSAA2X,
		MSAA4X,
		MSAA8X,
		MSAA16X
	};

	enum ThumbnailResolution
	{
		MNSY_THUMBNAILRES_64	= 0,
		MNSY_THUMBNAILRES_128	= 1,
		MNSY_THUMBNAILRES_256	= 2,
		MNSY_THUMBNAILRES_512	= 3,
		MNSY_THUMBNAILRES_COUNT

	};

	enum RenderModes
	{
		MNSY_RENDERMODE_SHADED				= 0,
		MNSY_RENDERMODE_ALBEDO				= 1,
		MNSY_RENDERMODE_ROUGHNESS			= 2,
		MNSY_RENDERMODE_METALLIC			= 3,
		MNSY_RENDERMODE_NORMAL				= 4,
		MNSY_RENDERMODE_AMBIENTOCCLUSION	= 5,
		MNSY_RENDERMODE_EMISSION			= 6,
		MNSY_RENDERMODE_HEIGHT				= 7,
		MNSY_RENDERMODE_OPACITY				= 8
	};

	class Renderer
	{
	public:
		Renderer()  = default;
		~Renderer() = default;

		void Init();
		void Shutdown();

		void BindFramebuffer();
		void UnbindFramebuffer();
		void ResizeFramebuffer(unsigned int width, unsigned int height);
		unsigned int GetRenderTextureId();

		void SetPbrShaderBrdfLutUniforms();
		void SetPbrShaderLightUniforms(Light& light);
		void SetShaderSkyboxUniforms(Skybox& skybox);


		void SetProjectionMatrix(const glm::mat4& projectionMatrix);

		void SetViewMatrix(const glm::mat4& viewMatrix);

		void SetClearColor(float r, float g, float b);
		void ClearFrame();

		void StartFrame(unsigned int width, unsigned int height);
		void EndFrame(unsigned int width, unsigned int height);

		void RenderMeshes(RenderMesh& renderMesh, Shader* shader);
		void RenderGizmo(RenderMesh& renderMesh);
		void RenderLightMesh(Light& light);
		void RenderSkybox(Skybox& skybox);

		void RenderScene(Scene& scene, systems::LibEntryType materialType);

		void RenderThumbnail_PbrMaterial(PbrMaterial& activeMaterial);
		void RenderThumbnail_UnlitMaterial(UnlitMaterial* unlitMaterial);
		void RenderThumbnail_SkyboxMaterial(Skybox& unlitMaterial);


		unsigned int GetThumbnailRenderTextureID() { return m_thumb_blitTexture_ID; }


		int GetMSAAEnumAsInt() { return (int)m_msaaSamplesSettings; }
		void SetMSAASamples(const MSAAsamples& samples);
		
		unsigned int GetThumbnailResolutionValue(ThumbnailResolution thumbnailResolution);
		void SetThumbnailResolution(ThumbnailResolution thumbnailResolution) { m_thumbnailResolution = thumbnailResolution; }
		ThumbnailResolution GetThumbnailResolutionEnum() { return m_thumbnailResolution; }
		
		int GetCurrentRenderModeInt() { return (int)m_renderMode; }
		void SetRenderMode(RenderModes mode);

		void HotReloadPbrShader(float deltaSeconds);

	private:

		void LoadUserSettings();
		void SaveUserSettings();


		void CreateRenderingFramebuffer(unsigned int width, unsigned int height);
		void CreateBlitFramebuffer(unsigned int width, unsigned int height);
		void CreateThumbnailFramebuffers();
		int GetMSAAIntValue();

		// for rendering with msaa enabled
		unsigned int m_MSAA_FBO = 0;
		unsigned int m_MSAA_RBO = 0;
		unsigned int m_MSAA_renderTexture_ID = 0;

		unsigned int m_standard_FBO = 0;
		unsigned int m_standard_RBO = 0;
		unsigned int m_standard_renderTexture_ID = 0;


		unsigned int m_blitFBO = 0;
		unsigned int m_blitRenderTexture_ID;


		glm::vec3 m_clearColor = glm::vec3(0.0f, 0.0f, 0.0f);

		glm::mat4 m_viewMatrix = glm::mat4(1.0f);
		glm::mat4 m_projectionMatrix = glm::mat4(1.0f);


		Shader* m_pPbrShader = nullptr;
		Shader* m_pUnlitTexturesShader = nullptr;
		Shader* m_pUnlitMaterialShader = nullptr;

		Shader* m_pLightShader = nullptr;
		Shader* m_pSkyboxShader = nullptr;

		//int m_msaaSamples = 4;
		MSAAsamples m_msaaSamplesSettings = MSAA4X;
		bool m_msaaOff = false;

		// Thumbnails
		ThumbnailResolution m_thumbnailResolution = ThumbnailResolution::MNSY_THUMBNAILRES_128;


		unsigned int m_thumb_MSAA_Value = 16;
		unsigned int m_thumb_MSAA_FBO = 0;
		unsigned int m_thumb_MSAA_RBO = 0;
		unsigned int m_thumb_MSAA_renderTexture_ID = 0;

		unsigned int m_thumb_blitFBO = 0;
		unsigned int m_thumb_blitTexture_ID = 0;
		
		RenderModes m_renderMode = MNSY_RENDERMODE_SHADED;

	private:

		core::FileWatcher m_shaderFileWatcher;
		core::FileWatcher m_shaderSkyboxFileWatcher;
		float m_fileWatchTimeDelta = 0.0f;

	};

} // mnemosy::graphics

#endif // !GRAPHICS_RENDERER_H
