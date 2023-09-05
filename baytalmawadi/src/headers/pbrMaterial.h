#ifndef PBR_MATERIAL_H
#define PBR_MATERIAL_H

#include <glad/glad.h> 

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <Shader.h>
#include <Texture.h>


class PbrMaterial
{
public:
	//Shader pbrShader;


	glm::vec3 baseColor = glm::vec3(1.0f, 1.0f, 1.0f);
	float baseRoughness = 0.5f;
	float baseMetallic = 0.0f;
	glm::vec3 baseEmission = glm::vec3(0.0f, 0.0f, 0.0f);

	float emissionStrength = 1.0f;
	float normalStrength = 1.0f;



	Texture albedoTexture;
	Texture normalTexture;
	Texture roughnessTexture;
	Texture metallicTexture;
	Texture emissiveTexture;
	Texture aoTexture;



	PbrMaterial()
	{


		albedoTexture.generateFromFile("textures/panel_albedo.png", true, true);
		normalTexture.generateFromFile("textures/panel_normal.png", true, true);
		roughnessTexture.generateFromFile("textures/panel_roughness.png", true, true);
		metallicTexture.generateFromFile("textures/panel_metallic.png", true, true);
		emissiveTexture.generateFromFile("textures/panel_emissive.png", true, true);
		aoTexture.generateFromFile("textures/panel_ao.png", true, true);
	}

	void setUniforms()
	{

		
	}

private:






};


#endif // !OBJECT_H
