#ifndef MATERIAL_H
#define MATERIAL_H


#include <string>
#include <vector>
#include <glm/glm.hpp>


namespace mnemosy::graphics {

	enum NormalMapFormat;
	enum PBRTextureType;
	enum ChannelPackComponent;
	class Texture;
	class Shader;
}

namespace mnemosy::graphics
{

	class Material
	{
	public:
		Material();
		~Material();

		std::string Name = "Mnemosy Default";
		
		glm::vec3 Albedo = glm::vec3(0.8f, 0.8f, 0.8f);
		glm::vec3 Emission = glm::vec3(0.0f, 0.0f, 0.0f);
		glm::vec2 UVTiling = glm::vec2(1.0f,1.0f);

		float Roughness = 0.5f;
		float Metallic = 0.0f;
		float EmissionStrength = 0.0f;

		float OpacityTreshhold = 0.5f;
		float HeightDepth = 1.0f;
		float MaxHeight = 0.0f;
		float NormalStrength = 1.0f; // not supported by shader at the moment

		NormalMapFormat NormalTextureFormat;
		bool IsSmoothnessTexture = false;
		bool UseEmissiveAsMask = false;

		bool HasPackedTextures = false;
		std::vector<std::string> PackedTexturesSuffixes;



		void setDefaults();

		void SetNormalMapFormat(const NormalMapFormat& format);
		void assignTexture(const PBRTextureType& pbrType, Texture* texture);
		void assignTexture(const PBRTextureType& pbrType, const std::string& filePath);
		void removeTexture(const PBRTextureType& pbrType);
		void setMaterialUniforms(Shader& shader);


		bool isAlbedoAssigned()		{ return m_pAlbedoTexture; }
		bool isNormalAssigned()		{ return m_pNormalTexture; }
		bool isRoughnessAssigned()	{ return m_pRoughnessTexture; }
		bool isMetallicAssigned()	{ return m_pMetallicTexture; }
		bool isEmissiveAssigned()	{ return m_pEmissiveTexture; }
		bool isAoAssigned()			{ return m_pAmbientOcclusionTexture; }
		bool isOpacityAssigned()	{ return m_pOpacityTexture; }
		bool isHeightAssigned()		{ return m_pHeightTexture; }


		// These DO NOT check if textures exist yet
		Texture& GetAlbedoTexture()		{ return *m_pAlbedoTexture; }
		Texture& GetNormalTexture()		{ return *m_pNormalTexture; }
		Texture& GetRoughnessTexture()	{ return *m_pRoughnessTexture; }
		Texture& GetMetallicTexture()	{ return *m_pMetallicTexture; }
		Texture& GetEmissiveTexture()	{ return *m_pEmissiveTexture; }
		Texture& GetAOTexture()			{ return *m_pAmbientOcclusionTexture; }
		Texture& GetOpacityTexture()	{ return *m_pOpacityTexture; }
		Texture& GetHeightTexture()		{ return *m_pHeightTexture; }



		bool SuffixExistsInPackedTexturesList(std::string& suffix);
		bool IsTextureTypeAssigned(const PBRTextureType& pbrType);



		Texture* GetTextureFromPackComponent(ChannelPackComponent packComponent);
		float GetDefaultValueFromPackComponent(ChannelPackComponent packComponent);


		int GetNormalFormatAsInt() { return (int)NormalTextureFormat; }
		unsigned int DebugGetTextureID(const PBRTextureType& pbrType);


	private:
		Texture* m_pAlbedoTexture = nullptr;
		Texture* m_pNormalTexture = nullptr;
		Texture* m_pRoughnessTexture = nullptr;
		Texture* m_pMetallicTexture = nullptr;
		Texture* m_pEmissiveTexture = nullptr;
		Texture* m_pAmbientOcclusionTexture = nullptr;
		Texture* m_pOpacityTexture = nullptr;
		Texture* m_pHeightTexture = nullptr;
		
	};

} // mnemosy::graphics

#endif // !MATERIAL_H
