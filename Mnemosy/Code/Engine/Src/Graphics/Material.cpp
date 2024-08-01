#include "Include/Graphics/Material.h"

#include "Include/Core/Log.h"

#include "Include/Graphics/TextureDefinitions.h"
#include "Include/Graphics/Shader.h"
#include "Include/Graphics/Texture.h"



namespace mnemosy::graphics
{
	Material::Material()
	{
		setDefaults();
	}

	Material::~Material() {

		if (m_pAlbedoTexture) {
			delete m_pAlbedoTexture;
			m_pAlbedoTexture = nullptr;
		}

		if (m_pRoughnessTexture) {
			delete m_pRoughnessTexture;
			m_pRoughnessTexture = nullptr;
		}

		if (m_pMetallicTexture) {
			delete m_pMetallicTexture;
			m_pMetallicTexture = nullptr;
		}

		if (m_pNormalTexture) {
			delete m_pNormalTexture;
			m_pNormalTexture = nullptr;
		}

		if (m_pEmissiveTexture) {
			delete m_pEmissiveTexture;
			m_pEmissiveTexture = nullptr;
		}

		if (m_pAmbientOcclusionTexture) {
			delete m_pAmbientOcclusionTexture;
			m_pAmbientOcclusionTexture = nullptr;
		}

		if (m_pOpacityTexture) {
			delete m_pOpacityTexture;
			m_pOpacityTexture = nullptr;
		}

		if (m_pHeightTexture) {
			delete m_pHeightTexture;
			m_pHeightTexture = nullptr;
		}

		if (!PackedTexturesSuffixes.empty()) {
			PackedTexturesSuffixes.clear();
		}
	}

	void Material::setDefaults() {

		Name = "Mnemosy Default";
		Albedo = glm::vec3(0.8f, 0.8f, 0.8f);
		Roughness = 0.1f;
		IsSmoothnessTexture = false;
		Metallic = 0.0f;
		Emission = glm::vec3(0.0f, 0.0f, 0.0f);
		EmissionStrength = 0.0f;
		UseEmissiveAsMask = false;
		NormalStrength = 1.0f;
		HeightDepth = 0.0f;
		UVTiling = glm::vec2(1.0f, 1.0f);
		NormalTextureFormat = MNSY_NORMAL_FORMAT_OPENGl;
		OpacityTreshhold = 0.5f;

		HasPackedTextures = false;
	}

	void Material::SetNormalMapFormat(const NormalMapFormat& format) {

		NormalTextureFormat = format;
	}

	void Material::assignTexture(const PBRTextureType& pbrTextureType,Texture* tex) {

		MNEMOSY_ASSERT(pbrTextureType != MNSY_TEXTURE_COUNT, "Do not use this function to add custom packed textures");


		switch (pbrTextureType)
		{
		case MNSY_TEXTURE_ALBEDO:

			if (m_pAlbedoTexture) {
				delete m_pAlbedoTexture;
			}
			m_pAlbedoTexture = tex;

			break;
		case MNSY_TEXTURE_ROUGHNESS:

			if (m_pRoughnessTexture) {
				delete m_pRoughnessTexture;
			}
			m_pRoughnessTexture = tex;

			break;
		case MNSY_TEXTURE_METALLIC:

			if (m_pMetallicTexture) {
				delete m_pMetallicTexture;
			}
			m_pMetallicTexture = tex;

			break;
		case MNSY_TEXTURE_NORMAL:

			if (m_pNormalTexture) {
				delete m_pNormalTexture;
			}
			m_pNormalTexture = tex;

			break;
		case MNSY_TEXTURE_AMBIENTOCCLUSION:

			if (m_pAmbientOcclusionTexture) {
				delete m_pAmbientOcclusionTexture;
			}
			m_pAmbientOcclusionTexture = tex;

			break;
		case MNSY_TEXTURE_EMISSION:

			if (m_pEmissiveTexture) {
				delete m_pEmissiveTexture;
			}
			m_pEmissiveTexture = tex;

			break;
		case MNSY_TEXTURE_HEIGHT:

			if (m_pHeightTexture) {
				delete m_pHeightTexture;
			}
			m_pHeightTexture = tex;

			break;
		case MNSY_TEXTURE_OPACITY:

			if (m_pOpacityTexture) {
				delete m_pOpacityTexture;
			}
			m_pOpacityTexture = tex;

			break;

		}

	}

	void Material::assignTexture(const PBRTextureType& pbrType, const std::string& filePath) {

		MNEMOSY_ASSERT(pbrType != MNSY_TEXTURE_COUNT, "Do not use this function to add custom packed textures");

		Texture* tex = new Texture();
		bool loadedSuccesfull = tex->GenerateFromFile(filePath.c_str(),true,true);

		if (!loadedSuccesfull) {

			tex->clear();
			delete tex;
			tex = nullptr;
			return;
		}

		assignTexture(pbrType, tex);

	}

	void Material::removeTexture(const PBRTextureType& pbrTextureType) {
		
		switch (pbrTextureType)
		{
		case MNSY_TEXTURE_ALBEDO:

			if (m_pAlbedoTexture) {
				delete m_pAlbedoTexture;
				m_pAlbedoTexture = nullptr;
			}
			break;
		case MNSY_TEXTURE_ROUGHNESS:

			if (m_pRoughnessTexture) {
				delete m_pRoughnessTexture;
				m_pRoughnessTexture = nullptr;
			}
			IsSmoothnessTexture = false;
			break;
		case MNSY_TEXTURE_METALLIC:

			if (m_pMetallicTexture) {
				delete m_pMetallicTexture;
				m_pMetallicTexture = nullptr;
			}
			break;
		case MNSY_TEXTURE_NORMAL:

			if (m_pNormalTexture) {
				delete m_pNormalTexture;
				m_pNormalTexture = nullptr;
			}
			SetNormalMapFormat(MNSY_NORMAL_FORMAT_OPENGl);
			break;
		case MNSY_TEXTURE_AMBIENTOCCLUSION:

			if (m_pAmbientOcclusionTexture) {
				delete m_pAmbientOcclusionTexture;
				m_pAmbientOcclusionTexture = nullptr;
			}
			break;
		case MNSY_TEXTURE_EMISSION:

			if (m_pEmissiveTexture) {
				delete m_pEmissiveTexture;
				m_pEmissiveTexture = nullptr;
			}
			break;
		case MNSY_TEXTURE_HEIGHT:

			if (m_pHeightTexture) {
				delete m_pHeightTexture;
				m_pHeightTexture = nullptr;
			}
			break;
		case MNSY_TEXTURE_OPACITY:

			if (m_pOpacityTexture) {
				delete m_pOpacityTexture;
				m_pOpacityTexture = nullptr;
			}
			break;
		}

	}

	void Material::setMaterialUniforms(Shader& shader) {

		
		shader.Use();

		// set value inputs

		//shader.SetUniformFloat("_normalStrength", NormalStrength);
		shader.SetUniformFloat("_emissionStrength", EmissionStrength);
		shader.SetUniformFloat2("_uvTiling", UVTiling.x, UVTiling.y);
		shader.SetUniformFloat("_heightDepth", HeightDepth);
		
		// for the solid non texture values im passing an extra parameters for each as the last one to specify how much of it will contribute between the texture and the solid non texture values
		// esentially lerping between texture input and non texture input. the lerp value however is just binary 0 or 1 
		// the result is that if any texture is not bound it will use the base non texture value but if it is bound the the texture will be used
		// Its kind of a lot of setup but it works and avoids the use of if statements in the shader and also saves a bit on the amount of uniforms in the shader.
		// texture uniforms
		
		
		if (m_pAlbedoTexture) 
		{
		
			m_pAlbedoTexture->BindToLocation(0);
			shader.SetUniformFloat4("_albedoColorValue", Albedo.r, Albedo.g, Albedo.b,0.0f);
		}
		else
		{
			shader.SetUniformFloat4("_albedoColorValue", Albedo.r, Albedo.g, Albedo.b, 1.0f);
		}

		if (m_pNormalTexture)
		{
			m_pNormalTexture->BindToLocation(1);
			shader.SetUniformFloat2("_normalValue", NormalStrength,0.0f);
		}
		else
		{
			shader.SetUniformFloat2("_normalValue", NormalStrength,1.0f);
		}

		if (m_pRoughnessTexture)
		{
			m_pRoughnessTexture->BindToLocation(2);
			shader.SetUniformFloat2("_roughnessValue", Roughness, 0.0f);
		}
		else
		{
			shader.SetUniformFloat2("_roughnessValue", Roughness, 1.0f);
		}

		if (m_pMetallicTexture)
		{
			m_pMetallicTexture->BindToLocation(3);
			shader.SetUniformFloat2("_metallicValue", Metallic, 0.0f);
		}
		else 
		{
			shader.SetUniformFloat2("_metallicValue", Metallic, 1.0f);
		}


		if (m_pAmbientOcclusionTexture)
		{
			m_pAmbientOcclusionTexture->BindToLocation(4);
			shader.SetUniformFloat("_ambientOcculusionValue", 0.0f);
		}
		else
		{
			shader.SetUniformFloat("_ambientOcculusionValue", 1.0f);
		}

		if (m_pEmissiveTexture)
		{
			m_pEmissiveTexture->BindToLocation(5);
			shader.SetUniformFloat4("_emissionColorValue", Emission.r, Emission.g, Emission.b,0.0f);
		}
		else
		{
			shader.SetUniformFloat4("_emissionColorValue", Emission.r, Emission.g, Emission.b, 1.0f);
		}
		shader.SetUniformBool("_useEmissiveMapAsMask", UseEmissiveAsMask);

		if (m_pHeightTexture) {

			m_pHeightTexture->BindToLocation(6);
			shader.SetUniformBool("_heightAssigned", true);
		}
		else {
			shader.SetUniformBool("_heightAssigned", false);
		}

		if (m_pOpacityTexture) {
			m_pOpacityTexture->BindToLocation(7);
			shader.SetUniformFloat2("_opacityValue",OpacityTreshhold , 0.0f);
		}
		else {
			shader.SetUniformFloat2("_opacityValue", OpacityTreshhold ,1.0f);
		}



		shader.SetUniformInt("_albedoMap", 0);
		shader.SetUniformInt("_normalMap", 1);
		shader.SetUniformInt("_roughnessMap", 2);
		shader.SetUniformInt("_metallicMap", 3);
		shader.SetUniformInt("_ambientOcculusionMap", 4);
		shader.SetUniformInt("_emissionMap", 5);
		shader.SetUniformInt("_heightMap", 6);
		shader.SetUniformInt("_opacityMap", 7);

	}



	bool Material::SuffixExistsInPackedTexturesList(std::string& suffix) {

		if (!HasPackedTextures)
			return false;

		if (PackedTexturesSuffixes.empty())
			return false;


		for (int i = 0; i < PackedTexturesSuffixes.size(); i++) {

			if (suffix == PackedTexturesSuffixes[i]) {
				return true;
			}
		}

		return false;
	}

	bool Material::IsTextureTypeAssigned(const PBRTextureType& pbrType) {

		switch (pbrType)
		{
		case mnemosy::graphics::MNSY_TEXTURE_ALBEDO:

			return isAlbedoAssigned();
			break;
		case mnemosy::graphics::MNSY_TEXTURE_ROUGHNESS:
			return isRoughnessAssigned();
			break;
		case mnemosy::graphics::MNSY_TEXTURE_METALLIC:
			return isMetallicAssigned();
			break;
		case mnemosy::graphics::MNSY_TEXTURE_NORMAL:
			return isNormalAssigned();
			break;
		case mnemosy::graphics::MNSY_TEXTURE_AMBIENTOCCLUSION:
			return isAoAssigned();
			break;
		case mnemosy::graphics::MNSY_TEXTURE_EMISSION:
			return isEmissiveAssigned();
			break;
		case mnemosy::graphics::MNSY_TEXTURE_HEIGHT:
			return isHeightAssigned();
			break;
		case mnemosy::graphics::MNSY_TEXTURE_OPACITY:
			return isOpacityAssigned();
			break;
		case mnemosy::graphics::MNSY_TEXTURE_COUNT:
			return false;
			break;
		default:
			return false;
			break;
		}

		return false;
	}


	// returns nullptr if texture is not assigned
	Texture* Material::GetTextureFromPackComponent(ChannelPackComponent packComponent) {

		switch (packComponent)
		{
		case (MNSY_PACKCOMPONENT_NONE):
			return nullptr;
			break;
		case (MNSY_PACKCOMPONENT_ALBEDO_R):
			return m_pAlbedoTexture;
			break;
		case (MNSY_PACKCOMPONENT_ALBEDO_G):
			return m_pAlbedoTexture;
			break;
		case (MNSY_PACKCOMPONENT_ALBEDO_B):
			return m_pAlbedoTexture;
			break;


		case (MNSY_PACKCOMPONENT_NORMAL_R):
			return m_pNormalTexture;
			break;
		case (MNSY_PACKCOMPONENT_NORMAL_G):
			return m_pNormalTexture;
			break;

		case (MNSY_PACKCOMPONENT_NORMAL_B):
			return m_pNormalTexture;
			break;


		case (MNSY_PACKCOMPONENT_EMISSIVE_R):
			return m_pEmissiveTexture;
			break;
		case (MNSY_PACKCOMPONENT_EMISSIVE_G):
			return m_pEmissiveTexture;
			break;
		case (MNSY_PACKCOMPONENT_EMISSIVE_B):
			return m_pEmissiveTexture;
			break;


		case (MNSY_PACKCOMPONENT_ROUGHNESS):
			return m_pRoughnessTexture;
			break;
		case (MNSY_PACKCOMPONENT_METALLIC):
			return m_pMetallicTexture;
			break;
		case (MNSY_PACKCOMPONENT_AO):
			return m_pAmbientOcclusionTexture;
			break;
		case (MNSY_PACKCOMPONENT_HEIGHT):
			return m_pHeightTexture;
			break;

		case (MNSY_PACKCOMPONENT_OPACITY):
			return m_pOpacityTexture;
			break;

		default:
			return nullptr;
			break;
		}

		return nullptr;
	}

	unsigned int Material::DebugGetTextureID(const PBRTextureType& pbrTextureType)
	{
		if (pbrTextureType == MNSY_TEXTURE_ALBEDO)
		{
			if(m_pAlbedoTexture)
				return m_pAlbedoTexture->GetID();
		}
		else if (pbrTextureType == MNSY_TEXTURE_ROUGHNESS)
		{
			if(m_pRoughnessTexture)
				return m_pRoughnessTexture->GetID();
		}
		else if (pbrTextureType == MNSY_TEXTURE_METALLIC)
		{
			if(m_pMetallicTexture)
				return m_pMetallicTexture->GetID();
		}
		else if (pbrTextureType == MNSY_TEXTURE_NORMAL)
		{
			if(m_pNormalTexture)
				return m_pNormalTexture->GetID();
		}
		else if (pbrTextureType == MNSY_TEXTURE_AMBIENTOCCLUSION)
		{
			if(m_pAmbientOcclusionTexture)
				return m_pAmbientOcclusionTexture->GetID();
		}
		else if (pbrTextureType == MNSY_TEXTURE_EMISSION)
		{
			if(m_pEmissiveTexture)
				return m_pEmissiveTexture->GetID();
		}
		else if (pbrTextureType == MNSY_TEXTURE_HEIGHT) {
			if (m_pHeightTexture)
				return m_pHeightTexture->GetID();
		}
		else if (pbrTextureType == MNSY_TEXTURE_OPACITY) {
			if (m_pOpacityTexture)
				return m_pOpacityTexture->GetID();
		}

		return 0;
	}

} // !mnemosy::graphics

