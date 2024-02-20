#ifndef MESH_RENDERER_H
#define MESH_RENDERER_H

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <view/scene/DefaultScene.h>
#include <model/Object.h>
#include <view/Cubemap.h>
#include <view/pbrMaterial.h>



class SceneRenderer
{
public:
	SceneRenderer(unsigned int width, unsigned int height);
	~SceneRenderer();



	void BindFramebuffer();
	void UnbindFramebuffer();
	void RescaleFramebuffer(unsigned int width, unsigned int height);
	unsigned int GetRenderTextureId();


	void SetPbrShaderGlobalSceneUniforms(Shader& pbrShader, Cubemap& cubemap, glm::vec3 lightPosition,float lightStrength, glm::vec3 cameraPosition, float skyboxRotation);
	void SetProjectionMatrix(glm::mat4 projectionMatrix);
	void SetViewMatrix(glm::mat4 viewMatrix);

	void ClearFrame(float r, float g, float b);
	void StarFrame(unsigned int width, unsigned int height);
	void EndFrame();
	

	void RenderMesh(Object& object,PbrMaterial& material);
	void RenderSkybox(Object& object, Shader& skyboxShader, Cubemap& cubemap,float rotation, glm::vec3 colorTint);
	
	
	void RenderScene(DefaultScene* activeScene, unsigned int viewportWidth, unsigned int viewportHeight);
	
private:
	void CreateFramebuffer(unsigned int width, unsigned int height);

	unsigned int FrameBufferObject;
	unsigned int RenderBufferObject;
	unsigned int RenderTexture_Id;

	glm::mat4 m_viewMatrix = glm::mat4(1.0f);
	glm::mat4 m_projectionMatrix = glm::mat4(1.0f);

};

#endif // !MESH_RENDERER_H
