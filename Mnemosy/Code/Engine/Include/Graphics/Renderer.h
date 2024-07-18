#ifndef GRAPHICS_RENDERER_H
#define GRAPHICS_RENDERER_H

#include "Include/Core/Utils/FileWatcher.h"
#include <glm/glm.hpp>

#include <filesystem>



namespace mnemosy::graphics
{
	class Shader;
	class RenderMesh;
	class Skybox;
	class Material;
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

	class Renderer
	{
	public:
		Renderer();
		~Renderer();


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

		void RenderMeshes(RenderMesh& renderMesh);
		void RenderGizmo(RenderMesh& renderMesh);
		void RenderLightMesh(Light& light);
		void RenderSkybox(Skybox& skybox);

		void RenderScene(Scene& scene);
		void RenderThumbnail(Material& activeMaterial);
		unsigned int GetThumbnailRenderTextureID() { return m_thumb_blitTexture_ID; }


		int GetMSAA() { return (int)m_msaaSamplesSettings; }
		void SetMSAASamples(const MSAAsamples& samples);
		unsigned int GetThumbnailResolution() { return m_thumbnailResolution; };
		
		
		void HotReloadPbrShader(float deltaSeconds);
	private:
		void CreateRenderingFramebuffer(unsigned int width, unsigned int height);
		void CreateBlitFramebuffer(unsigned int width, unsigned int height);

		void CreateThumbnailFramebuffers();


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
		Shader* m_pLightShader = nullptr;
		Shader* m_pSkyboxShader = nullptr;
		Shader* m_pGizmoShader = nullptr;

		int m_msaaSamples = 4;
		MSAAsamples m_msaaSamplesSettings = MSAA4X;
		bool m_msaaOff = false;

		// Thumbnails
		unsigned int m_thumbnailResolution = 200;

		unsigned int m_thumb_MSAA_FBO = 0;
		unsigned int m_thumb_MSAA_RBO = 0;
		unsigned int m_thumb_MSAA_renderTexture_ID = 0;

		unsigned int m_thumb_blitFBO = 0;
		unsigned int m_thumb_blitTexture_ID = 0;
		

	private:

		core::FileWatcher m_shaderFileWatcher;
		float m_fileWatchTimeDelta = 0.0f;

	};

} // mnemosy::graphics

#endif // !GRAPHICS_RENDERER_H
