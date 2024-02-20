#include "Engine/Include/Graphics/Material.h"

#include "Engine/Include/Core/Log.h"

//#include "Engine/Engine.h"
//#include <glad/glad.h> 
//#include <gtc/matrix_transform.hpp>

namespace mnemosy::graphics
{

	Material::Material()
	{
		setDefaults();
	}

	Material::~Material()
	{
		//delete pbrShader;

		//delete m_pPbrShader;
		//m_pPbrShader = nullptr;


		delete m_pAlbedoTexture;
		delete m_pNormalTexture;
		delete m_pRoughnessTexture;
		delete m_pMetallicTexture;
		delete m_pEmissiveTexture;
		delete m_pAmbientOcclusionTexture;
		
		m_pNormalTexture = nullptr;
		m_pRoughnessTexture = nullptr;
		m_pMetallicTexture = nullptr;
		m_pAlbedoTexture = nullptr;
		m_pEmissiveTexture = nullptr;
		m_pAmbientOcclusionTexture = nullptr;
	}

	void Material::setDefaults()
	{
		Albedo = glm::vec3(1.0f, 1.0f, 1.0f);
		Roughness = 0.5f;
		Metallic = 0.0f;
		Emission = glm::vec3(0.0f, 0.0f, 0.0f);
		EmissionStrength = 0.0f;
		NormalStrength = 1.0f; // currently not supported by shader
	}

	//void Material::assignShader(Shader& shader)
	//{
	//	m_pPbrShader = &shader;
	//}

	void Material::assignTexture(PBRTextureType pbrTextureType,Texture& texture)
	{
		if (pbrTextureType == ALBEDO)
			m_pAlbedoTexture = &texture;
		else if (pbrTextureType == ROUGHNESS)
			m_pRoughnessTexture = &texture;
		else if (pbrTextureType == METALLIC)
			m_pMetallicTexture = &texture;
		else if (pbrTextureType == NORMAL)
			m_pNormalTexture = &texture;
		else if (pbrTextureType == AMBIENTOCCLUSION)
			m_pAmbientOcclusionTexture = &texture;
		else if (pbrTextureType == EMISSION)
			m_pEmissiveTexture = &texture;
	}

	void Material::assignTexture(PBRTextureType pbrType, std::string filePath)
	{

		if (pbrType == ALBEDO)
		{
			if(!m_pAlbedoTexture)
				m_pAlbedoTexture = new Texture();
			
			m_pAlbedoTexture->generateFromFile(filePath.c_str(), true,true);
			// probably better to handle it through an asset registry
			// m_pAlbedoTexture = Engine::GetInstance()->GetAssetRegistry().LoadTextureGL(filePath);
			return;
		}

		if (pbrType == ROUGHNESS)
		{
			if (!m_pRoughnessTexture)
				m_pRoughnessTexture = new Texture();

			m_pRoughnessTexture->generateFromFile(filePath.c_str(), true, true);
			return;
		}
	
		if (pbrType == METALLIC)
		{
			if (!m_pMetallicTexture)
				m_pMetallicTexture = new Texture();

			m_pMetallicTexture->generateFromFile(filePath.c_str(), true, true);
			return;
		}
		if (pbrType == NORMAL)
		{
			if (!m_pNormalTexture)
				m_pNormalTexture = new Texture();

			m_pNormalTexture->generateFromFile(filePath.c_str(), true, true);
			return;
		}
		if (pbrType == AMBIENTOCCLUSION)
		{
			if (!m_pAmbientOcclusionTexture)
				m_pAmbientOcclusionTexture = new Texture();

			m_pAmbientOcclusionTexture->generateFromFile(filePath.c_str(), true, true);
			return;
		}
		if (pbrType == EMISSION)
		{
			if (!m_pEmissiveTexture)
				m_pEmissiveTexture = new Texture();

			m_pEmissiveTexture->generateFromFile(filePath.c_str(), true, true);
			return;
		}
	}

	void Material::removeTexture(PBRTextureType pbrTextureType)
	{
		if (pbrTextureType == ALBEDO)
		{
			delete m_pAlbedoTexture;
			m_pAlbedoTexture = nullptr;
		}
		else if (pbrTextureType == ROUGHNESS)
		{
			delete m_pRoughnessTexture;
			m_pRoughnessTexture = nullptr;

		}
		else if (pbrTextureType == METALLIC)
		{
			delete m_pMetallicTexture;
			m_pMetallicTexture = nullptr;
		}
		else if (pbrTextureType == NORMAL)
		{
			delete m_pNormalTexture;
			m_pNormalTexture = nullptr;
		}
		else if (pbrTextureType == AMBIENTOCCLUSION)
		{
			delete m_pAmbientOcclusionTexture;
			m_pAmbientOcclusionTexture = nullptr;
		}
		else if (pbrTextureType == EMISSION)
		{
			delete m_pEmissiveTexture;
			m_pEmissiveTexture = nullptr;
		}
	}

	void Material::setMaterialUniforms(Shader& shader)
	{
		/*MNEMOSY_ASSERT(m_pPbrShader, "Material has no shader assigned");
		if (!m_pPbrShader)
			return;*/
		
		//shader.Use();
		shader.Use();

		// set value inputs

		shader.SetUniformFloat("_normalStrength", NormalStrength); // currently not supported by shader
		shader.SetUniformFloat("_emissionStrength", EmissionStrength);

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
			shader.SetUniformFloat("_normalValue", 0.0f);
		}
		else
		{
			shader.SetUniformFloat("_normalValue", 1.0f);
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
			m_pEmissiveTexture->BindToLocation(6);
			shader.SetUniformFloat4("_emissionColorValue", Emission.r, Emission.g, Emission.b,0.0f);
		}
		else
		{
			shader.SetUniformFloat4("_emissionColorValue", Emission.r, Emission.g, Emission.b, 1.0f);
		}


		shader.SetUniformInt("_albedoMap", 0);
		shader.SetUniformInt("_normalMap", 1);
		shader.SetUniformInt("_roughnessMap", 2);
		shader.SetUniformInt("_metallicMap", 3);
		shader.SetUniformInt("_ambientOcculusionMap", 4);
		shader.SetUniformInt("_emissionMap", 6);
	}

	//Shader& Material::GetShader()
	//{
	//	return *m_pPbrShader;
	//}


}

