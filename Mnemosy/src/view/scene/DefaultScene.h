#pragma once
//#include <view/scene/Scene.h>

#include <glm/glm.hpp>

#include <controller/ModelLoader.h>

#include <model/Camera.h>
#include <model/Object.h>

#include <view/Cubemap.h>
#include <view/pbrMaterial.h>
#include <view/Shader.h>
#include <view/Texture.h>


class DefaultScene 
{
public:
	DefaultScene() = default;
	DefaultScene(unsigned int ScreenWidth, unsigned int ScreenHeight)
	{
		Init(ScreenWidth,ScreenHeight);
	}
	~DefaultScene() = default;

	void Init(unsigned int ScreenWidth, unsigned int ScreenHeight) 
	{
		environmentRotation = 0.0f;
		skyboxColorTint = glm::vec3(1, 1, 1);

		camera = new Camera(ScreenWidth,ScreenHeight);
		camera->position = glm::vec3(0.0f, 0.0f, 3.0f);


		ModelLoader modelLoader;


		pbrShader.CreateShaderProgram("src/shaders/pbrVertex.vert", "src/shaders/pbrFragment.frag");
		skyboxShader.CreateShaderProgram("src/shaders/skybox.vert", "src/shaders/skybox.frag");

		//Shader pbrShader("src/shaders/pbrVertex.vert", "src/shaders/pbrFragment.frag");
		//Shader skyboxShader("src/shaders/skybox.vert", "src/shaders/skybox.frag");

		// == Skybox 
		//skyboxObject = new Object();
		skyboxObject.modelData = modelLoader.LoadModelDataFromFile("fbx/skyboxMesh.fbx");

		//environmentTexture = new Cubemap("textures/market.hdr", 1024, true);
		environmentTexture.loadFromFile("textures/market.hdr", 1024, true);


		// == Base Mesh
		//baseObject = new Object();
		baseObject.modelData = modelLoader.LoadModelDataFromFile("fbx/TestObj.fbx");
		baseObject.position = glm::vec3(0.0f, 0.0f, 0.0f);
		baseObject.rotation.x = -90.0f;


		//pbrMaterial = new PbrMaterial();
		pbrMaterial.assignShader(pbrShader);
		pbrMaterial.assignTexture(ALBEDO, "textures/PikachuTexture.png");
		pbrMaterial.assignTexture(ROUGHNESS, "textures/panel_roughness.png");
		pbrMaterial.assignTexture(METALLIC, "textures/panel_metallic.png");
		pbrMaterial.assignTexture(NORMAL, "textures/brick_normal.png");
		pbrMaterial.assignTexture(AMBIENTOCCLUSION, "textures/panel_ao.png");
		pbrMaterial.assignTexture(EMISSION, "textures/panel_emissive.png");

		pbrMaterial.EmissionStrength = 0;
		pbrMaterial.Albedo = glm::vec3(0.3f, 0.3f, 0.3f);
		pbrMaterial.Emission = glm::vec3(0.0f, 0.3f, 0.7f);
		pbrMaterial.Metallic = 0.0f;
		pbrMaterial.Roughness = 0.4f;



		// == Light Object
		//lightObject = new Object();
		
		lightObject.modelData = modelLoader.LoadModelDataFromFile("fbx/ArrowZ.fbx");
		lightObject.position = glm::vec3(0.0f, 1.0f, 0.0f);
		lightObject.rotation = glm::vec3(45.0f, 0.0f, 0.0f);
		lightObject.scale = glm::vec3(0.1f, 0.1f, 0.1f);


		//lightMaterial = new PbrMaterial();
		lightMaterial.assignShader(pbrShader);
		lightMaterial.Albedo = glm::vec3(1.0f, 1.0f, 1.0f);
		lightMaterial.Roughness = 1.0;
		lightMaterial.Emission = glm::vec3(1.0f, 1.0f, 1.0f);
		lightMaterial.EmissionStrength = 8.0f;
	}

	void Update()
	{
		lightObject.rotation.y = glm::degrees(environmentRotation);
	}

public:
	Shader pbrShader;
	Shader skyboxShader;

	Camera* camera;

	Object baseObject;
	Object lightObject;
	Object skyboxObject;

	PbrMaterial pbrMaterial;
	PbrMaterial lightMaterial;


	Cubemap environmentTexture;

	// Global Scene Variables
	float environmentRotation = 0.0f;
	glm::vec3 skyboxColorTint = glm::vec3(1, 1, 1);
};