#ifndef GRAPHICS_RENDERER_H
#define GRAPHICS_RENDERER_H

#include <glm/glm.hpp>

namespace mnemosy::graphics
{
	class Shader;
	class RenderMesh;
	class Skybox;
	class Light;
	class Scene;
}


namespace mnemosy::graphics
{
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
		void SetPbrShaderLightUniforms();
		void SetShaderSkyboxUniforms();


		void SetProjectionMatrix(glm::mat4 projectionMatrix);

		void SetViewMatrix(glm::mat4 viewMatrix);

		void SetClearColor(float r, float g, float b);
		void ClearFrame();

		void StartFrame(unsigned int width, unsigned int height);
		void EndFrame(unsigned int width, unsigned int height);

		void RenderMeshes(RenderMesh& renderMesh);
		void RenderLightMesh(Light& light);
		void RenderSkybox(Skybox& skybox);

		void RenderScene(Scene& scene);

	private:
		void CreateRenderingFramebuffer(unsigned int width, unsigned int height);
		void CreateBlitFramebuffer(unsigned int width, unsigned int height);

		// for rendering with msaa enabled
		unsigned int m_frameBufferObject = 0;
		unsigned int m_renderBufferObject = 0;
		unsigned int m_renderTexture_Id = 0;

		unsigned int m_blitFbo = 0;
		unsigned int m_blitedRenderTexture_Id;


		glm::vec3 m_clearColor = glm::vec3(0.0f, 0.0f, 0.0f);

		glm::mat4 m_viewMatrix = glm::mat4(1.0f);
		glm::mat4 m_projectionMatrix = glm::mat4(1.0f);


		Shader* m_pPbrShader = nullptr;
		Shader* m_pLightShader = nullptr;
		Shader* m_pSkyboxShader = nullptr;


		int msaaSamples = 4;
	};



}

#endif // !GRAPHICS_RENDERER_H