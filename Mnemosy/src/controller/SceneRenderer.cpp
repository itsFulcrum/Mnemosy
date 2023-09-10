#include "SceneRenderer.h"



SceneRenderer::SceneRenderer()
{

}
SceneRenderer::~SceneRenderer()
{

}

void SceneRenderer::SetProjectionMatrix(glm::mat4 projectionMatrix)
{
	this->m_projectionMatrix = projectionMatrix;
}
void SceneRenderer::SetViewMatrix(glm::mat4 viewMatrix)
{
	this->m_viewMatrix = viewMatrix;
}

void SceneRenderer::ClearFrame(float r, float g,float b)
{
	// rendering commands
	glClearColor(r, g, b, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}


void SceneRenderer::SetPbrShaderGlobalSceneUniforms(Shader& pbrShader, Cubemap& cubemap, glm::vec3 lightPosition,glm::vec3 cameraPosition)
{
	pbrShader.use();
	pbrShader.setUniformFloat3("_lightPositionWS", lightPosition.x, lightPosition.y, lightPosition.z);
	pbrShader.setUniformFloat("_lightStrength", 1);
	pbrShader.setUniformFloat3("_cameraPositionWS", cameraPosition.x, cameraPosition.y, cameraPosition.z);

	cubemap.BindIrradianceCubemap(7);
	pbrShader.setUniformInt("_irradianceMap", 7);
	cubemap.BindPrefilteredCubemap(8);
	pbrShader.setUniformInt("_prefilterMap", 8);
	cubemap.BindBrdfLutTexture(9);
	pbrShader.setUniformInt("_brdfLUT", 9);
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

void SceneRenderer::RenderSkybox(Object& object, Shader& skyboxShader,Cubemap& cubemap)
{
	// consider using a mesh that has faces point inwards to make this call obsolete
	glCullFace(GL_BACK);
	glDepthFunc(GL_LEQUAL);

	skyboxShader.use();


	cubemap.BindColorCubemap(0);
	skyboxShader.setUniformInt("_skybox", 0);
	
	glm::mat4 skyboxViewMatrix = glm::mat4(glm::mat3(m_viewMatrix));
	skyboxShader.setUniformMatrix4("_viewMatrix", skyboxViewMatrix);
	skyboxShader.setUniformMatrix4("_projectionMatrix", m_projectionMatrix);


	for (unsigned int i = 0; i < object.modelData.meshes.size(); i++)
	{
		glBindVertexArray(object.modelData.meshes[i].vertexArrayObject);
		glDrawElements(GL_TRIANGLES, object.modelData.meshes[i].indecies.size(), GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
	}


	glDepthFunc(GL_LESS);
	glCullFace(GL_FRONT);
}