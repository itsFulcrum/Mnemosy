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



		glDeleteFramebuffers(1, &m_blitFbo);
		glDeleteTextures(1, &m_blitedRenderTexture_Id);

		glDeleteRenderbuffers(1, &m_renderBufferObject);
		glDeleteFramebuffers(1, &m_frameBufferObject);
		glDeleteTextures(1, &m_renderTexture_Id);
	}

	// bind renderFrameBuffer
	void Renderer::BindFramebuffer()
	{
		MNEMOSY_ASSERT(m_frameBufferObject != 0, "Framebuffer has not be created yet");
		glBindFramebuffer(GL_FRAMEBUFFER, m_frameBufferObject);
	}

	void Renderer::UnbindFramebuffer()
	{
		glBindRenderbuffer(GL_RENDERBUFFER, 0);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void Renderer::ResizeFramebuffer(unsigned int width, unsigned int height)
	{
		
		// resize multisampled render framebuffer
		glBindFramebuffer(GL_FRAMEBUFFER, m_frameBufferObject);
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, m_renderTexture_Id);


		glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, m_msaaSamples, GL_RGB, width, height, GL_TRUE);
		//glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		//glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, m_renderTexture_Id, 0);

		glBindRenderbuffer(GL_RENDERBUFFER, m_renderBufferObject);
		glRenderbufferStorageMultisample(GL_RENDERBUFFER,m_msaaSamples, GL_DEPTH24_STENCIL8, width, height);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_renderBufferObject);
		

		// resize intermediate blit framebuffer
		glBindFramebuffer(GL_FRAMEBUFFER,m_blitFbo);
		glBindTexture(GL_TEXTURE_2D, m_blitedRenderTexture_Id);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_blitedRenderTexture_Id, 0);
	}

	unsigned int Renderer::GetRenderTextureId()
	{

		return m_blitedRenderTexture_Id;
		//return m_renderTexture_Id;
	}

	
	void Renderer::SetPbrShaderBrdfLutUniforms()
	{
		m_pPbrShader->Use();
		// we should probably only have to do this once not every frame
		MnemosyEngine::GetInstance().GetIblRenderer().BindBrdfLutTexture(9);
		m_pPbrShader->SetUniformInt("_brdfLUT", 9);

	}

	void Renderer::SetPbrShaderLightUniforms()
	{

		Light& light = MnemosyEngine::GetInstance().GetScene().GetLight();

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

	void Renderer::SetShaderSkyboxUniforms()
	{
		Skybox& skybox = MnemosyEngine::GetInstance().GetScene().GetSkybox();

		m_pPbrShader->Use();
		skybox.GetCubemap().BindIrradianceCubemap(7);
		m_pPbrShader->SetUniformInt("_irradianceMap", 7);
		skybox.GetCubemap().BindPrefilteredCubemap(8);
		m_pPbrShader->SetUniformInt("_prefilterMap", 8);
		m_pPbrShader->SetUniformFloat("_skyboxExposure", skybox.exposure);
		m_pPbrShader->SetUniformFloat("_skyboxRotation", skybox.rotation);


		m_pSkyboxShader->Use();

		// test if we only need to do this once;
		skybox.GetCubemap().BindColorCubemap(0);
		m_pSkyboxShader->SetUniformInt("_skybox", 0);
		skybox.GetCubemap().BindIrradianceCubemap(1);
		m_pSkyboxShader->SetUniformInt("_irradianceMap", 1);
		skybox.GetCubemap().BindPrefilteredCubemap(2);
		m_pSkyboxShader->SetUniformInt("_prefilterMap", 2);


		m_pSkyboxShader->SetUniformFloat("_rotation", skybox.rotation);
		m_pSkyboxShader->SetUniformFloat3("_colorTint", skybox.colorTint.r, skybox.colorTint.g, skybox.colorTint.b);
		m_pSkyboxShader->SetUniformFloat("_exposure", skybox.exposure);

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
		//MNEMOSY_DEBUG("Renderer::ClearFrame")
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

	void Renderer::EndFrame(unsigned int width, unsigned int height)
	{
		// now resolve multisampled buffer(s) into intermediate FBO
		// we dont have to bind m_frameBufferObject here again because it is already bound at framestart.
		//glBindFramebuffer(GL_READ_FRAMEBUFFER, m_frameBufferObject);
		
		
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_blitFbo);
		glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_NEAREST);

		// now scene is stored as 2D texture image, so use that image for post-processing

		UnbindFramebuffer();
	}

	void Renderer::RenderMeshes(RenderMesh& renderMesh)
	{
		m_pPbrShader->Use();
		renderMesh.GetMaterial().setMaterialUniforms(*m_pPbrShader);

		glm::mat4 modelMatrix = renderMesh.transform.GetTransformMatrix();


		m_pPbrShader->SetUniformMatrix4("_modelMatrix", modelMatrix);
		m_pPbrShader->SetUniformMatrix4("_normalMatrix", renderMesh.transform.GetNormalMatrix(modelMatrix));
		m_pPbrShader->SetUniformMatrix4("_viewMatrix", m_viewMatrix);
		m_pPbrShader->SetUniformMatrix4("_projectionMatrix", m_projectionMatrix);

		for (unsigned int i = 0; i < renderMesh.GetModelData().meshes.size(); i++)
		{
			glBindVertexArray(renderMesh.GetModelData().meshes[i].vertexArrayObject);
			glDrawElements(GL_TRIANGLES, (GLsizei)renderMesh.GetModelData().meshes[i].indecies.size(), GL_UNSIGNED_INT, 0);
			glBindVertexArray(0);
		}

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
			glBindVertexArray(0);
		}
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
			glBindVertexArray(0);
		}

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
			glBindVertexArray(0);
		}

		glDepthFunc(GL_LESS);
		glCullFace(GL_FRONT);
	}

	void Renderer::RenderScene(Scene& scene)
	{
		unsigned int width = MnemosyEngine::GetInstance().GetWindow().GetViewportWidth();
		unsigned int height = MnemosyEngine::GetInstance().GetWindow().GetViewportHeight();

		StartFrame(width, height);

		//SetPbrShaderGlobalSceneUniforms(scene.GetSkybox(), scene.GetLight(), scene.GetCamera().transform.GetPosition());

		m_pPbrShader->Use();
		glm::vec3 cameraPosition = scene.GetCamera().transform.GetPosition();
		m_pPbrShader->SetUniformFloat3("_cameraPositionWS", cameraPosition.x, cameraPosition.y, cameraPosition.z);

		RenderMeshes(scene.GetMesh());
		//RenderMeshes(scene.GetGizmoMesh());
		RenderGizmo(scene.GetGizmoMesh());
		RenderLightMesh(scene.GetLight());
		RenderSkybox(scene.GetSkybox());

		EndFrame(width,height);

	}

	void Renderer::SetMSAASamples(const MSAAsamples& samples)
	{
		m_msaaOff = false;
		if (samples == MSAAOFF)
		{
			//glDisable(GL_MULTISAMPLE);
			m_msaaOff = true;
			m_msaaSamples = 2; // this causes crash
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
	void Renderer::CreateRenderingFramebuffer(unsigned int width, unsigned int height)
	{
		glGenFramebuffers(1, &m_frameBufferObject);
		glBindFramebuffer(GL_FRAMEBUFFER, m_frameBufferObject);

		glGenTextures(1, &m_renderTexture_Id);
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, m_renderTexture_Id);
		glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, m_msaaSamples , GL_RGB, width, height, GL_TRUE);
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, m_renderTexture_Id, 0);
		// dont work with multisampling
		//glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		//glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glGenRenderbuffers(1, &m_renderBufferObject);
		glBindRenderbuffer(GL_RENDERBUFFER, m_renderBufferObject);
		glRenderbufferStorageMultisample(GL_RENDERBUFFER,m_msaaSamples, GL_DEPTH24_STENCIL8, width, height);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_renderBufferObject);

		MNEMOSY_ASSERT(glad_glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "Faild to complete framebuffer");
		
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);

		MNEMOSY_DEBUG("Renderer: Framebuffer created");

	}

	void Renderer::CreateBlitFramebuffer(unsigned int width, unsigned int height)
	{
		glGenFramebuffers(1, &m_blitFbo);
		glBindFramebuffer(GL_FRAMEBUFFER, m_blitFbo);


		glGenTextures(1, &m_blitedRenderTexture_Id);
		glBindTexture(GL_TEXTURE_2D,m_blitedRenderTexture_Id);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height,0,GL_RGB,GL_UNSIGNED_BYTE,nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_blitedRenderTexture_Id, 0);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

} // !mnemosy::graphics