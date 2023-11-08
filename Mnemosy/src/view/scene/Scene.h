#pragma once

#include <glm/glm.hpp>

#include <controller/ModelLoader.h>

#include <model/Camera.h>
#include <model/Object.h>

#include <view/Cubemap.h>
#include <view/pbrMaterial.h>
#include <view/Shader.h>
#include <view/Texture.h>


class Scene
{
public:
	
	virtual ~Scene() =  default;

	virtual void Init(unsigned int ScreenWidth,unsigned int ScreenHeight) = 0;
	virtual void Update() = 0;

	Shader pbrShader;
	Shader skyboxShader;

	Camera* camera;

	Object* baseObject;
	Object* lightObject;
	Object* skyboxObject;

	PbrMaterial* pbrMaterial;
	PbrMaterial* lightMaterial;


	Cubemap* environmentTexture;

	// Global Scene Variables
	float environmentRotation = 0.0f;
	glm::vec3 skyboxColorTint = glm::vec3(1, 1, 1);

};