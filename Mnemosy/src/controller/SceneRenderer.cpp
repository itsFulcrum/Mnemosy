#include "SceneRenderer.h"



SceneRenderer::SceneRenderer(unsigned int width, unsigned int height)
{
	CreateFramebuffer(width, height);
}
SceneRenderer::~SceneRenderer()
{

}

void SceneRenderer::CreateFramebuffer(unsigned int width, unsigned int height)
{
	glGenFramebuffers(1, &FrameBufferObject);
	glBindFramebuffer(GL_FRAMEBUFFER, FrameBufferObject);

	glGenTextures(1, &RenderTexture_Id);
	glBindTexture(GL_TEXTURE_2D, RenderTexture_Id);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, RenderTexture_Id, 0);

	glGenRenderbuffers(1, &RenderBufferObject);
	glBindRenderbuffer(GL_RENDERBUFFER, RenderBufferObject);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, RenderBufferObject);

	if (glad_glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cout << "ERROR::SCENE_RENDERER::" << "createFramebuffer faild to complete." << std::endl;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
}

void SceneRenderer::BindFramebuffer()
{
	glBindFramebuffer(GL_FRAMEBUFFER, FrameBufferObject);
}
void SceneRenderer::UnbindFramebuffer() 
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
void SceneRenderer::RescaleFramebuffer(unsigned int width, unsigned int height)
{
	glBindTexture(GL_TEXTURE_2D, RenderTexture_Id);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,RenderTexture_Id, 0);

	glBindRenderbuffer(GL_RENDERBUFFER, RenderBufferObject);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, RenderBufferObject);
}

unsigned int SceneRenderer::GetRenderTextureId()
{
	return RenderTexture_Id;
}

void SceneRenderer::SetProjectionMatrix(glm::mat4 projectionMatrix)
{
	this->m_projectionMatrix = projectionMatrix;
}
void SceneRenderer::SetViewMatrix(glm::mat4 viewMatrix)
{
	this->m_viewMatrix = viewMatrix;
}


void SceneRenderer::StarFrame(unsigned int width, unsigned int height)
{
	RescaleFramebuffer(width, height);
	glViewport(0, 0, width, height);
	BindFramebuffer();
	glViewport(0, 0, width, height);
	
	ClearFrame(0.0f, 0.0f, 0.0f);
}
void SceneRenderer::EndFrame()
{
	UnbindFramebuffer();
}
void SceneRenderer::ClearFrame(float r, float g,float b)
{
	// rendering commands
	glClearColor(r, g, b, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void SceneRenderer::SetPbrShaderGlobalSceneUniforms(Shader& pbrShader, Cubemap& cubemap, glm::vec3 lightPosition, float lightStrength, glm::vec3 cameraPosition, float skyboxRotation)
{
	pbrShader.use();
	pbrShader.setUniformFloat3("_lightPositionWS", lightPosition.x, lightPosition.y, lightPosition.z);
	pbrShader.setUniformFloat("_lightStrength",lightStrength);
	pbrShader.setUniformFloat3("_cameraPositionWS", cameraPosition.x, cameraPosition.y, cameraPosition.z);

	cubemap.BindIrradianceCubemap(7);
	pbrShader.setUniformInt("_irradianceMap", 7);
	cubemap.BindPrefilteredCubemap(8);
	pbrShader.setUniformInt("_prefilterMap", 8);
	cubemap.BindBrdfLutTexture(9);
	pbrShader.setUniformInt("_brdfLUT", 9);

	pbrShader.setUniformFloat("_skyboxRotation", skyboxRotation);
	
}

void SceneRenderer::RenderMesh(Object& object, PbrMaterial& material)
{
	//object.m_model.m_meshes.size();
	//shader.use();

	material.setMaterialUniforms();


	glm::mat4 modelMatrix = object.GetTransformMatrix();

	material.pbrShader->setUniformMatrix4("_modelMatrix", modelMatrix);
	material.pbrShader->setUniformMatrix4("_normalMatrix", object.GetNormalMatrix(modelMatrix));
	material.pbrShader->setUniformMatrix4("_viewMatrix", m_viewMatrix);
	material.pbrShader->setUniformMatrix4("_projectionMatrix", m_projectionMatrix);

	// object.ModelData.meshes

	for (unsigned int i = 0; i < object.modelData.meshes.size(); i++)
	{
		
		//m_meshes[i].DrawMesh(shader);
		glBindVertexArray(object.modelData.meshes[i].vertexArrayObject);
		glDrawElements(GL_TRIANGLES, object.modelData.meshes[i].indecies.size(), GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
	}


}
void SceneRenderer::RenderSkybox(Object& object, Shader& skyboxShader,Cubemap& cubemap,float rotation, glm::vec3 colorTint)
{
	// consider using a mesh that has faces point inwards to make this call obsolete
	glCullFace(GL_BACK);
	glDepthFunc(GL_LEQUAL);

	skyboxShader.use();

	glm::mat4 skyboxViewMatrix = glm::mat4(glm::mat3(m_viewMatrix));
	skyboxShader.setUniformMatrix4("_viewMatrix", skyboxViewMatrix);
	skyboxShader.setUniformMatrix4("_projectionMatrix", m_projectionMatrix);

	skyboxShader.setUniformFloat("_rotation", rotation);

	// fragment
	cubemap.BindColorCubemap(0);
	skyboxShader.setUniformInt("_skybox", 0);
	skyboxShader.setUniformFloat3("_tint", colorTint.r,colorTint.g,colorTint.b);
	


	for (unsigned int i = 0; i < object.modelData.meshes.size(); i++)
	{
		glBindVertexArray(object.modelData.meshes[i].vertexArrayObject);
		glDrawElements(GL_TRIANGLES, object.modelData.meshes[i].indecies.size(), GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
	}


	glDepthFunc(GL_LESS);
	glCullFace(GL_FRONT);
}
void SceneRenderer::RenderScene(DefaultScene* activeScene, unsigned int viewportWidth, unsigned int viewportHeight)
{
	activeScene->camera.updateScreenSize(viewportWidth, viewportHeight);



	SetProjectionMatrix(activeScene->camera.GetProjectionMatrix());
	SetViewMatrix(activeScene->camera.GetViewMatrix());

	//StarFrame(viewportWidth,viewportHeight);

	ClearFrame(0.0f, 0.0f, 0.0f);

	// Uniforms have to be set after start Frame
	SetPbrShaderGlobalSceneUniforms(activeScene->pbrShader, activeScene->environmentTexture, -activeScene->lightObject.GetForward(), activeScene->lightMaterial.EmissionStrength, activeScene->camera.position, activeScene->environmentRotation);

	// draw calls
	RenderMesh(activeScene->baseObject, activeScene->pbrMaterial);
	// render light source
	RenderMesh(activeScene->lightObject, activeScene->lightMaterial);
	// Render skybox last
	RenderSkybox(activeScene->skyboxObject, activeScene->skyboxShader, activeScene->environmentTexture, activeScene->environmentRotation, activeScene->skyboxColorTint);


	//EndFrame();

}

