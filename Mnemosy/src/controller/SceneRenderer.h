#ifndef MESH_RENDERER_H
#define MESH_RENDERER_H

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <model/Object.h>
#include <view/Cubemap.h>
#include <view/pbrMaterial.h>



class SceneRenderer
{
public:
	SceneRenderer();
	~SceneRenderer();

	void ClearFrame(float r, float g, float b);
	void RenderMesh(Object& object,PbrMaterial& material);
	void RenderSkybox(Object& object, Shader& skyboxShader, Cubemap& cubemap);
	
	void SetPbrShaderGlobalSceneUniforms(Shader& pbrShader, Cubemap& cubemap, glm::vec3 lightPosition, glm::vec3 cameraPosition);
	void SetProjectionMatrix(glm::mat4 projectionMatrix);
	void SetViewMatrix(glm::mat4 viewMatrix);

private:
	glm::mat4 m_viewMatrix = glm::mat4(1.0f);
	glm::mat4 m_projectionMatrix = glm::mat4(1.0f);

};

#endif // !MESH_RENDERER_H
