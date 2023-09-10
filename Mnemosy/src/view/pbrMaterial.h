#ifndef PBR_MATERIAL_H
#define PBR_MATERIAL_H

#include <glad/glad.h> 

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <view/Shader.h>
#include <view/Texture.h>

enum PBRTextureType
{
	ALBEDO,
	ROUGHNESS,
	METALLIC,
	NORMAL,
	AMBIENTOCCLUSION,
	EMISSION
};

class PbrMaterial
{
public:

	PbrMaterial();
	~PbrMaterial();

	Shader* pbrShader = nullptr;


	glm::vec3 Albedo = glm::vec3(1.0f, 1.0f, 1.0f);
	glm::vec3 Emission = glm::vec3(1.0f, 0.0f, 0.0f);
	float Roughness = 0.5f;
	float Metallic = 1.0f;
	float EmissionStrength = 1.0f;
	// currently not supported by shader
	float NormalStrength = 1.0f;



	void assignShader(Shader& newShader);
	void assignTexture(PBRTextureType pbrType, std::string Filepath);
	void removeTexture(PBRTextureType pbrType);
	void setMaterialUniforms();

private:
	
	Texture albedoTexture;
	Texture normalTexture;
	Texture roughnessTexture;
	Texture metallicTexture;
	Texture emissiveTexture;
	Texture ambientOcclusionTexture;


};


#endif // !OBJECT_H
