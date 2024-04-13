#include "Include/Graphics/Renderer.h"
#include "Include/MnemosyEngine.h"

#include "Include/Core/Window.h"
#include "Include/Core/Log.h"
#include "Include/Core/FileDirectories.h"

#include "Include/Graphics/ImageBasedLightingRenderer.h"
#include "Include/Graphics/Camera.h"
#include "Include/Graphics/Shader.h"
#include "Include/Graphics/Material.h"
#include "Include/Graphics/ModelData.h"
#include "Include/Graphics/RenderMesh.h"
#include "Include/Graphics/Cubemap.h"
#include "Include/Graphics/Skybox.h"
#include "Include/Graphics/Light.h"
#include "Include/Graphics/Scene.h"
#include "Include/Graphics/ThumbnailScene.h"


#include <filesystem>
#include <glad/glad.h>

namespace mnemosy::graphics
{
	// public

	Renderer::Renderer() 
	{
		MnemosyEngine& engine = MnemosyEngine::GetInstance();

		std::filesystem::path shaders = engine.GetFileDirectories().GetShadersPath();
		std::string shadersPath = shaders.generic_string() + "/";
		std::string pbrVert		= shadersPath + "pbrVertex.vert";
		std::string pbrFrag		= shadersPath + "pbrFragment.frag";
		//std::string pbrMesh		= shadersPath + "pbrVert_MESH.glsl";
		//std::string pbrMeshFrag	= shadersPath + "pbrMeshFragment.frag";
		std::string lightVert	= shadersPath + "light.vert";
		std::string lightFrag	= shadersPath + "light.frag";
		std::string skyboxVert	= shadersPath + "skybox.vert";
		std::string skyboxFrag	= shadersPath + "skybox.frag";
		std::string gizmoVert	= shadersPath + "gizmo.vert";
		std::string gizmoFrag	= shadersPath + "gizmo.frag";


		m_pPbrShader	= new Shader(pbrVert.c_str(), pbrFrag.c_str());

		m_pLightShader	= new Shader(lightVert.c_str(), lightFrag.c_str());
		m_pSkyboxShader = new Shader(skyboxVert.c_str(), skyboxFrag.c_str());
		m_pGizmoShader	= new Shader(gizmoVert.c_str(),gizmoFrag.c_str());

		unsigned int w = engine.GetWindow().GetWindowWidth();
		unsigned int h = engine.GetWindow().GetWindowHeight();
		CreateRenderingFramebuffer(w,h);
		CreateBlitFramebuffer(w,h);
		CreateThumbnailFramebuffers();

		//SetPbrShaderBrdfLutUniforms();
	}
	Renderer::~Renderer()
	{
		

		delete m_pPbrShader;
		delete m_pLightShader;
		delete m_pSkyboxShader;
		delete m_pGizmoShader;
		m_pPbrShader = nullptr;
		m_pLightShader = nullptr;
		m_pSkyboxShader = nullptr;
		m_pGizmoShader = nullptr;


		// deleting MSAA Framebuffer stuff
		glDeleteRenderbuffers(1, &m_MSAA_RBO);
		glDeleteFramebuffers(1, &m_MSAA_FBO);
		glDeleteTextures(1, &m_MSAA_renderTexture_ID);

		glDeleteFramebuffers(1, &m_blitFBO);
		glDeleteTextures(1, &m_blitRenderTexture_ID);

		// deleting standard framebuffer stuff (no MSAA)
		glDeleteFramebuffers(1, &m_standard_FBO);
		glDeleteRenderbuffers(1, &m_standard_RBO);
		glDeleteTextures(1, &m_standard_renderTexture_ID);
		
		// Delete thumbnail textures and framebuffers
		glDeleteFramebuffers(1, &m_thumb_MSAA_FBO);
		glDeleteRenderbuffers(1, &m_thumb_MSAA_RBO);

		glDeleteTextures(1, &m_thumb_MSAA_renderTexture_ID);
		glDeleteFramebuffers(1, &m_thumb_blitFBO);
		glDeleteTextures(1, &m_thumb_blitTexture_ID);
	}

	// bind renderFrameBuffer
	void Renderer::BindFramebuffer() {

		if (m_msaaOff) {

			MNEMOSY_ASSERT(m_standard_FBO != 0, "Framebuffer has not be created yet");
			glBindFramebuffer(GL_FRAMEBUFFER, m_standard_FBO);			
			return;
		}

		MNEMOSY_ASSERT(m_MSAA_FBO != 0, "Framebuffer has not be created yet");
		glBindFramebuffer(GL_FRAMEBUFFER, m_MSAA_FBO);
	}

	void Renderer::UnbindFramebuffer() {

		glBindRenderbuffer(GL_RENDERBUFFER, 0);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void Renderer::ResizeFramebuffer(unsigned int width, unsigned int height)
	{

		if (m_msaaOff) { // resizing standard framebuffer and texture
			
			glBindFramebuffer(GL_FRAMEBUFFER, m_standard_FBO);
			glBindTexture(GL_TEXTURE_2D, m_standard_renderTexture_ID);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			glBindRenderbuffer(GL_RENDERBUFFER, m_standard_RBO);
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
			
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			return;
		}

		// Resizing multisampled (MSAA) framebuffers and textures
		glBindFramebuffer(GL_FRAMEBUFFER, m_MSAA_FBO);
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, m_MSAA_renderTexture_ID);
		glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, m_msaaSamples, GL_RGB, width, height, GL_TRUE);

		glBindRenderbuffer(GL_RENDERBUFFER, m_MSAA_RBO);
		glRenderbufferStorageMultisample(GL_RENDERBUFFER,m_msaaSamples, GL_DEPTH24_STENCIL8, width, height);

		// resize intermediate blit framebuffer
		glBindFramebuffer(GL_FRAMEBUFFER,m_blitFBO);
		glBindTexture(GL_TEXTURE_2D, m_blitRenderTexture_ID);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}

	unsigned int Renderer::GetRenderTextureId() {

		if (m_msaaOff) {
			return m_standard_renderTexture_ID;
		}
		return m_blitRenderTexture_ID;
	}

	
	void Renderer::SetPbrShaderBrdfLutUniforms()
	{
		m_pPbrShader->Use();
		MnemosyEngine::GetInstance().GetIblRenderer().BindBrdfLutTexture(9);
		m_pPbrShader->SetUniformInt("_brdfLUT", 9);

	}

	void Renderer::SetPbrShaderLightUniforms(Light& light)
	{

		//Light& light = MnemosyEngine::GetInstance().GetScene().GetLight();

		m_pPbrShader->Use();

		// depending on light type we pass light forward vector into light position and for point lights the actual position
		if (light.GetLightType() == LightType::DIRECTIONAL) {

			glm::vec3 lightForward = light.transform.GetForward();
			m_pPbrShader->SetUniformFloat3("_lightPositionWS", lightForward.x, lightForward.y, lightForward.z);
		}
		else if (light.GetLightType() == LightType::POINT)
		{
			glm::vec3 lightPosition = light.transform.GetPosition();
			m_pPbrShader->SetUniformFloat3("_lightPositionWS", lightPosition.x, lightPosition.y, lightPosition.z);
		}

		m_pPbrShader->SetUniformFloat("_lightStrength", light.strength);
		m_pPbrShader->SetUniformFloat3("_lightColor", light.color.r, light.color.g, light.color.b);
		m_pPbrShader->SetUniformInt("_lightType", light.GetLightTypeAsInt());
		m_pPbrShader->SetUniformFloat("_lightAttentuation", light.falloff);
	}

	void Renderer::SetShaderSkyboxUniforms(Skybox& skybox) {
		
		m_pPbrShader->Use();
		
		skybox.GetCubemap().BindIrradianceCubemap(7);
		m_pPbrShader->SetUniformInt("_irradianceMap", 7);
		skybox.GetCubemap().BindPrefilteredCubemap(8);
		m_pPbrShader->SetUniformInt("_prefilterMap", 8);
		m_pPbrShader->SetUniformFloat("_skyboxExposure", skybox.exposure);
		m_pPbrShader->SetUniformFloat("_skyboxRotation", skybox.rotation);


		m_pSkyboxShader->Use();

		skybox.GetCubemap().BindColorCubemap(0);
		m_pSkyboxShader->SetUniformInt("_skybox", 0);
		skybox.GetCubemap().BindIrradianceCubemap(1);
		m_pSkyboxShader->SetUniformInt("_irradianceMap", 1);
		skybox.GetCubemap().BindPrefilteredCubemap(2);
		m_pSkyboxShader->SetUniformInt("_prefilterMap", 2);


		m_pSkyboxShader->SetUniformFloat("_rotation", skybox.rotation);
		m_pSkyboxShader->SetUniformFloat3("_colorTint", skybox.colorTint.r, skybox.colorTint.g, skybox.colorTint.b);
		m_pSkyboxShader->SetUniformFloat("_exposure", skybox.exposure);

		m_pSkyboxShader->SetUniformFloat("_blurRadius", skybox.blurRadius);
		m_pSkyboxShader->SetUniformFloat3("_backgroundColor", skybox.backgroundColor.r, skybox.backgroundColor.g, skybox.backgroundColor.b);
		m_pSkyboxShader->SetUniformFloat("_gradientOpacity", skybox.gradientOpacity);
		m_pSkyboxShader->SetUniformFloat("_opacity", skybox.opacity);
		m_pSkyboxShader->SetUniformInt("_blurSteps", skybox.blurSteps);

	}

	void Renderer::SetProjectionMatrix(const glm::mat4& projectionMatrix)
	{
		m_projectionMatrix = projectionMatrix;
	}

	void Renderer::SetViewMatrix(const glm::mat4& viewMatrix)
	{
		m_viewMatrix = viewMatrix;
	}

	void Renderer::SetClearColor(float r, float g, float b)
	{
		m_clearColor.r = r;
		m_clearColor.g = g;
		m_clearColor.b = b;
	}

	void Renderer::ClearFrame()
	{
		glClearColor(m_clearColor.r, m_clearColor.g, m_clearColor.b, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		//glEnable(GL_DEPTH_TEST);
	}

	void Renderer::StartFrame(unsigned int width, unsigned int height)
	{
		int width_ = (int)width;
		int height_ = (int)height;

		glViewport(0, 0, width_, height_);
		ResizeFramebuffer(width_ , height_);

		//glViewport(0, 0, width_, height_);
		//glBindFramebuffer(GL_FRAMEBUFFER, m_frameBufferObject);
		
		BindFramebuffer();
		ClearFrame();
	}

	void Renderer::EndFrame(unsigned int width, unsigned int height) {

		if (!m_msaaOff) {

			// resolve multisampled buffer into intermediate blit FBO
			// we dont have to bind m_frameBufferObject here again because it is already bound at framestart.
			//glBindFramebuffer(GL_READ_FRAMEBUFFER, m_frameBufferObject);
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_blitFBO);
			glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
		}		
	
		UnbindFramebuffer();
	}

	void Renderer::RenderMeshes(RenderMesh& renderMesh) {

		glm::mat4 modelMatrix = renderMesh.transform.GetTransformMatrix();

		m_pPbrShader->Use();
		m_pPbrShader->SetUniformMatrix4("_modelMatrix", modelMatrix);
		m_pPbrShader->SetUniformMatrix4("_normalMatrix", renderMesh.transform.GetNormalMatrix(modelMatrix));
		m_pPbrShader->SetUniformMatrix4("_viewMatrix", m_viewMatrix);
		m_pPbrShader->SetUniformMatrix4("_projectionMatrix", m_projectionMatrix);

		for (unsigned int i = 0; i < renderMesh.GetModelData().meshes.size(); i++) {
			glBindVertexArray(renderMesh.GetModelData().meshes[i].vertexArrayObject);
			glDrawElements(GL_TRIANGLES, (GLsizei)renderMesh.GetModelData().meshes[i].indecies.size(), GL_UNSIGNED_INT, 0);
		}
		glBindVertexArray(0);

		// Mesh Shader Experiment
		/*
		glCullFace(GL_BACK);
		for (unsigned int i = 0; i < renderMesh.GetModelData().meshes.size(); i++) {

			for (unsigned int a = 0; a < renderMesh.GetModelData().meshes[i].vertecies.size(); a++) {

				m_pPbrShader->SetUniformFloat3("_VertPos",renderMesh.GetModelData().meshes[i].vertecies[a].position.x, renderMesh.GetModelData().meshes[i].vertecies[a].position.y, renderMesh.GetModelData().meshes[i].vertecies[a].position.z);
				glDrawMeshTasksNV(0,1);
			}
		}
		glCullFace(GL_FRONT);
		*/	

	}

	void Renderer::RenderGizmo(RenderMesh& renderMesh)
	{
		m_pGizmoShader->Use();
		glm::mat4 modelMatrix = renderMesh.transform.GetTransformMatrix();
		m_pGizmoShader->SetUniformMatrix4("_modelMatrix", modelMatrix);
		m_pGizmoShader->SetUniformMatrix4("_normalMatrix", renderMesh.transform.GetNormalMatrix(modelMatrix));
		m_pGizmoShader->SetUniformMatrix4("_viewMatrix", glm::mat4(glm::mat3(m_viewMatrix)));
		m_pGizmoShader->SetUniformMatrix4("_projectionMatrix", m_projectionMatrix);

		for (unsigned int i = 0; i < renderMesh.GetModelData().meshes.size(); i++)
		{
			glBindVertexArray(renderMesh.GetModelData().meshes[i].vertexArrayObject);
			glDrawElements(GL_TRIANGLES, (GLsizei)renderMesh.GetModelData().meshes[i].indecies.size(), GL_UNSIGNED_INT, 0);
		}
		glBindVertexArray(0);
	}

	void Renderer::RenderLightMesh(Light& light)
	{
		m_pLightShader->Use();

		m_pLightShader->SetUniformFloat("_lightStrength", light.strength);
		m_pLightShader->SetUniformFloat3("_lightColor", light.color.r, light.color.g, light.color.b);
		//m_pPbrShader->SetUniformInt("_lightType", light.GetLightTypeAsInt());
		//m_pPbrShader->SetUniformFloat("_lightAttentuation", light.falloff);



		glm::mat4 modelMatrix = light.transform.GetTransformMatrix();
		m_pLightShader->SetUniformMatrix4("_modelMatrix", modelMatrix);
		//m_pPbrShader->SetUniformMatrix4("_normalMatrix", renderMesh.transform.GetNormalMatrix(modelMatrix));
		m_pLightShader->SetUniformMatrix4("_viewMatrix", m_viewMatrix);
		m_pLightShader->SetUniformMatrix4("_projectionMatrix", m_projectionMatrix);

		for (unsigned int i = 0; i < light.GetModelData().meshes.size(); i++)
		{
			glBindVertexArray(light.GetModelData().meshes[i].vertexArrayObject);
			glDrawElements(GL_TRIANGLES, (GLsizei)light.GetModelData().meshes[i].indecies.size(), GL_UNSIGNED_INT, 0);
		}
		glBindVertexArray(0);

	}

	void Renderer::RenderSkybox(Skybox& skybox)
	{
		// consider using a mesh that has faces point inwards to make this call obsolete
		glCullFace(GL_BACK);
		glDepthFunc(GL_LEQUAL);

		m_pSkyboxShader->Use();

		glm::mat4 skyboxViewMatrix = glm::mat4(glm::mat3(m_viewMatrix));
		m_pSkyboxShader->SetUniformMatrix4("_viewMatrix", skyboxViewMatrix);
		m_pSkyboxShader->SetUniformMatrix4("_projectionMatrix", m_projectionMatrix);

		for (unsigned int i = 0; i < skybox.GetModelData().meshes.size(); i++)
		{
			glBindVertexArray(skybox.GetModelData().meshes[i].vertexArrayObject);
			glDrawElements(GL_TRIANGLES, (GLsizei)skybox.GetModelData().meshes[i].indecies.size(), GL_UNSIGNED_INT, 0);
		}
		glBindVertexArray(0);

		glDepthFunc(GL_LESS);
		glCullFace(GL_FRONT);
	}

	void Renderer::RenderScene(Scene& scene)
	{
		unsigned int width = MnemosyEngine::GetInstance().GetWindow().GetViewportWidth();
		unsigned int height = MnemosyEngine::GetInstance().GetWindow().GetViewportHeight();

		StartFrame(width, height);

		m_pPbrShader->Use();
		glm::vec3 cameraPosition = scene.GetCamera().transform.GetPosition();
		m_pPbrShader->SetUniformFloat3("_cameraPositionWS", cameraPosition.x, cameraPosition.y, cameraPosition.z);

		scene.GetActiveMaterial().setMaterialUniforms(*m_pPbrShader);

		RenderMeshes(scene.GetMesh());
		RenderGizmo(scene.GetGizmoMesh());
		RenderLightMesh(scene.GetLight());
		RenderSkybox(scene.GetSkybox());

		EndFrame(width,height);
	}

	void Renderer::RenderThumbnail(Material& activeMaterial) {

		unsigned int res = m_thumbnailResolution;
		ThumbnailScene& thumbScene = MnemosyEngine::GetInstance().GetThumbnailScene();
		
		// Setup Shaders with thumbnail Scene settings
		SetPbrShaderLightUniforms(thumbScene.GetLight());
		SetShaderSkyboxUniforms(thumbScene.GetSkybox());

		thumbScene.GetCamera().SetScreenSize(res, res); // we should really only need to do this once..

		m_projectionMatrix = thumbScene.GetCamera().GetProjectionMatrix();
		m_viewMatrix = thumbScene.GetCamera().GetViewMatrix();

		// Start Frame
		glViewport(0, 0,res, res);
		glBindFramebuffer(GL_FRAMEBUFFER, m_thumb_MSAA_FBO);

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		m_pPbrShader->Use();

		glm::vec3 cameraPosition = thumbScene.GetCamera().transform.GetPosition();
		m_pPbrShader->SetUniformFloat3("_cameraPositionWS", cameraPosition.x, cameraPosition.y, cameraPosition.z);

		activeMaterial.setMaterialUniforms(*m_pPbrShader);

		RenderMeshes(thumbScene.GetMesh());
		RenderSkybox(thumbScene.GetSkybox());

		// End Frame
		// blit msaa framebuffer to normal framebuffer
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER,m_thumb_blitFBO);
		glBlitFramebuffer(0, 0, res, res, 0, 0, res, res, GL_COLOR_BUFFER_BIT, GL_NEAREST);
		// unbind frambuffers
		glBindRenderbuffer(GL_RENDERBUFFER, 0);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);


		// Restore user pbr shader settings
		Scene& scene = MnemosyEngine::GetInstance().GetScene();
		SetPbrShaderLightUniforms(scene.GetLight());
		SetShaderSkyboxUniforms(scene.GetSkybox());
	}

	void Renderer::SetMSAASamples(const MSAAsamples& samples)
	{
		m_msaaOff = false;
		if (samples == MSAAOFF) {
			m_msaaOff = true;
		}
		else if (samples == MSAA2X) 
			m_msaaSamples = 2;
		else if (samples == MSAA4X)
			m_msaaSamples = 4;
		else if (samples == MSAA8X)
			m_msaaSamples = 8;
		else if (samples == MSAA16X)
			m_msaaSamples = 16;

		m_msaaSamplesSettings = samples;
	}

	// private
	void Renderer::CreateRenderingFramebuffer(unsigned int width, unsigned int height) {
		// MSAA FRAMEBUFFERS
		// Generate MSAA Framebuffer
		glGenFramebuffers(1, &m_MSAA_FBO);
		glBindFramebuffer(GL_FRAMEBUFFER, m_MSAA_FBO);
		// Generate MSAA Render Texture
		glGenTextures(1, &m_MSAA_renderTexture_ID);
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, m_MSAA_renderTexture_ID);
		glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, m_msaaSamples , GL_RGB, width, height, GL_TRUE);
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
		// bind MSAA render texture to MSAA framebuffer
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, m_MSAA_renderTexture_ID, 0);
		// dont work with multisampling
		//glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		//glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		// Generate MSAA Renderbuffer
		glGenRenderbuffers(1, &m_MSAA_RBO);
		glBindRenderbuffer(GL_RENDERBUFFER, m_MSAA_RBO);
		glRenderbufferStorageMultisample(GL_RENDERBUFFER,m_msaaSamples, GL_DEPTH24_STENCIL8, width, height);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_MSAA_RBO);

		MNEMOSY_ASSERT(glad_glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "Faild to complete MSAA framebuffer");

		// STANDARD FRAMEBUFFERS
		// Generate standart Framebuffer (no MSAA)
		glGenFramebuffers(1, &m_standard_FBO);
		glBindFramebuffer(GL_FRAMEBUFFER, m_standard_FBO);
		// Generate MSAA Render Texture
		glGenTextures(1, &m_standard_renderTexture_ID);
		glBindTexture(GL_TEXTURE_2D, m_standard_renderTexture_ID);
		glTexImage2D(GL_TEXTURE_2D,0, GL_RGB, width, height,0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
		glBindTexture(GL_TEXTURE_2D, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		// bind standard render texture to standard framebuffer
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_standard_renderTexture_ID, 0);
	
		// Generate MSAA Renderbuffer
		glGenRenderbuffers(1, &m_standard_RBO);
		glBindRenderbuffer(GL_RENDERBUFFER, m_standard_RBO);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_standard_RBO);

		MNEMOSY_ASSERT(glad_glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "Faild to complete standard framebuffer");

		// unbind everything		
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
		glBindTexture(GL_TEXTURE_2D, 0);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);

		MNEMOSY_DEBUG("Renderer: Framebuffer created");

	}

	void Renderer::CreateBlitFramebuffer(unsigned int width, unsigned int height)
	{
		glGenFramebuffers(1, &m_blitFBO);
		glBindFramebuffer(GL_FRAMEBUFFER, m_blitFBO);


		glGenTextures(1, &m_blitRenderTexture_ID);
		glBindTexture(GL_TEXTURE_2D,m_blitRenderTexture_ID);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height,0,GL_RGB,GL_UNSIGNED_BYTE,nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_blitRenderTexture_ID, 0);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	void Renderer::CreateThumbnailFramebuffers() {

		const int thumbnailMSAA = 4;
		
		// Gen msaa framebuffer
		glGenFramebuffers(1, &m_thumb_MSAA_FBO);
		glBindFramebuffer(GL_FRAMEBUFFER, m_thumb_MSAA_FBO);

		// Generate msaa render texture
		glGenTextures(1, &m_thumb_MSAA_renderTexture_ID);
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, m_thumb_MSAA_renderTexture_ID);
		glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, thumbnailMSAA, GL_RGB, m_thumbnailResolution, m_thumbnailResolution, GL_TRUE);
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, m_thumb_MSAA_renderTexture_ID, 0);

		glGenRenderbuffers(1, &m_thumb_MSAA_RBO);
		glBindRenderbuffer(GL_RENDERBUFFER, m_thumb_MSAA_RBO);
		glRenderbufferStorageMultisample(GL_RENDERBUFFER, thumbnailMSAA, GL_DEPTH24_STENCIL8, m_thumbnailResolution, m_thumbnailResolution);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_thumb_MSAA_RBO);

		// check if frambuffer complete
		MNEMOSY_ASSERT(glad_glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "Faild to complete framebuffer");

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0); // might be redundant call
		glBindRenderbuffer(GL_RENDERBUFFER, 0); // might be redundant call

		// generate standart texture and framebuffers for blitting msaa render output to
		glGenFramebuffers(1, &m_thumb_blitFBO);
		glBindFramebuffer(GL_FRAMEBUFFER, m_thumb_blitFBO);

		glGenTextures(1, &m_thumb_blitTexture_ID);
		glBindTexture(GL_TEXTURE_2D, m_thumb_blitTexture_ID);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_thumbnailResolution, m_thumbnailResolution, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_thumb_blitTexture_ID, 0);
		
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

} // !mnemosy::graphics