#ifndef GRAPHCIS_MATERIAL_H
#define GRAPHCIS_MATERIAL_H


#include <Engine/Include/Graphics/Shader.h>
#include <Engine/Include/Graphics/Texture.h>

#include <glm/glm.hpp>

namespace mnemosy::graphics
{

	enum PBRTextureType
	{
		ALBEDO,
		ROUGHNESS,
		METALLIC,
		NORMAL,
		AMBIENTOCCLUSION,
		EMISSION
	};

	class Material
	{
	public:

		Material();
		~Material();

		glm::vec3 Albedo = glm::vec3(1.0f, 1.0f, 1.0f);
		glm::vec3 Emission = glm::vec3(0.0f, 0.0f, 0.0f);
		float Roughness = 0.5f;
		float Metallic = 0.0f;
		float EmissionStrength = 0.0f;
		// currently not supported by shader
		float NormalStrength = 1.0f;


		void setDefaults();
		//void assignShader(Shader& newShader);
		void assignTexture(PBRTextureType pbrType, Texture& texture);
		void assignTexture(PBRTextureType pbrType, std::string filePath);
		void removeTexture(PBRTextureType pbrType);
		void setMaterialUniforms(Shader& shader);
		//Shader& GetShader();


	private:
		
		//Shader* m_pPbrShader = nullptr;

		Texture* m_pAlbedoTexture = nullptr;
		Texture* m_pNormalTexture = nullptr;
		Texture* m_pRoughnessTexture = nullptr;
		Texture* m_pMetallicTexture = nullptr;
		Texture* m_pEmissiveTexture = nullptr;
		Texture* m_pAmbientOcclusionTexture = nullptr;
		
	};

}

#endif // !GRAPHCIS_MATERIAL_H
