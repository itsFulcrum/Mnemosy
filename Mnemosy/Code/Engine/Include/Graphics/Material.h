#ifndef MATERIAL_H
#define MATERIAL_H

#include "Include/Graphics/Shader.h"
#include "Include/Graphics/Texture.h"

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
		float NormalStrength = 1.0f; // not supported by shader at the moment
		glm::vec2 UVTiling = glm::vec2(1.0f,1.0f);


		void setDefaults();

		void assignTexture(const PBRTextureType& pbrType, Texture& texture);
		void assignTexture(const PBRTextureType& pbrType, const std::string& filePath);
		void removeTexture(const PBRTextureType& pbrType);
		void setMaterialUniforms(Shader& shader);

	private:
		Texture* m_pAlbedoTexture = nullptr;
		Texture* m_pNormalTexture = nullptr;
		Texture* m_pRoughnessTexture = nullptr;
		Texture* m_pMetallicTexture = nullptr;
		Texture* m_pEmissiveTexture = nullptr;
		Texture* m_pAmbientOcclusionTexture = nullptr;
		
	};

} // mnemosy::graphics

#endif // !MATERIAL_H
