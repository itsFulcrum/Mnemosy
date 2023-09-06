#include <view/pbrMaterial.h>


PbrMaterial::PbrMaterial()
{
	Albedo = glm::vec3(1.0f, 1.0f, 1.0f);
	Roughness = 0.5f;
	Metallic = 1.0f;
	Emission = glm::vec3(1.0f, 1.0f, 0.0f);

	EmissionStrength = 1.0f;
	// currently not supported by shader
	NormalStrength = 1.0f;

	
}
PbrMaterial::~PbrMaterial()
{
	//delete pbrShader;
}

void PbrMaterial::assignShader(Shader& shader)
{
	this->pbrShader = &shader;
}

void PbrMaterial::assignTexture(PBRTextureType type,std::string filepath)
{
	if(type == ALBEDO)
		albedoTexture.generateFromFile(filepath.c_str(), true, true);
	else if(type == ROUGHNESS)
		roughnessTexture.generateFromFile(filepath.c_str(), true, true);
	else if (type == METALLIC)
		metallicTexture.generateFromFile(filepath.c_str(), true, true);
	else if (type == NORMAL)
		normalTexture.generateFromFile(filepath.c_str(), true, true);
	else if (type == AMBIENTOCCLUSION)
		ambientOcclusionTexture.generateFromFile(filepath.c_str(), true, true);
	else if (type == EMISSION)
		emissiveTexture.generateFromFile(filepath.c_str(), true, true);
}

void PbrMaterial::removeTexture(PBRTextureType type)
{
	if (type == ALBEDO)
	
		albedoTexture.clear();
	else if (type == ROUGHNESS)
		roughnessTexture.clear();
	else if (type == METALLIC)
		metallicTexture.clear();
	else if (type == NORMAL)
		normalTexture.clear();
	else if (type == AMBIENTOCCLUSION)
		ambientOcclusionTexture.clear();
	else if (type == EMISSION)
		emissiveTexture.clear();
}

void PbrMaterial::setMaterialUniforms()
{
	if (pbrShader == nullptr)
	{
		std::cout << "ERROR::PBR_MATERIAL::NO_SHADER_ASSIGNED" << std::endl;
		return;
	}

	pbrShader->use();

	// set value inputs
	
	
	
	// normal strength is not supported by shader atm
	pbrShader->setUniformFloat("_normalStrength", NormalStrength);
	pbrShader->setUniformFloat("_emissionStrength", EmissionStrength);

	// for the solid non texture values im passing an extra parameters for each as the last one to specify how much of it will contribute between the texture and the solid non texture values
	// esentially lerping between texture input and non texture input. the lerp value however is just binary 0 or 1 
	// the result is that if any texture is not bound it will use the base non texture value but if it is bound the the texture will be used
	// Its kind of a lot of setup but it works and avoids the use of if statements in the shader and also saves a bit on the amount of uniforms in the shader.
	// texture uniforms
	if (albedoTexture.containsData()) 
	{
		albedoTexture.BindToLocation(0);
		pbrShader->setUniformFloat4("_albedoColorValue", Albedo.r, Albedo.g, Albedo.b,0.0f);
	}
	else
	{
		pbrShader->setUniformFloat4("_albedoColorValue", Albedo.r, Albedo.g, Albedo.b, 1.0f);
	}

	if (normalTexture.containsData())
	{
		normalTexture.BindToLocation(1);
		pbrShader->setUniformFloat("_normalValue", 0.0f);
	}
	else
	{
		pbrShader->setUniformFloat("_normalValue", 1.0f);
	}

	if (roughnessTexture.containsData())
	{
		roughnessTexture.BindToLocation(2);
		pbrShader->setUniformFloat2("_roughnessValue", Roughness, 0.0f);
	}
	else
	{
		pbrShader->setUniformFloat2("_roughnessValue", Roughness, 1.0f);
	}

	if (metallicTexture.containsData())
	{
		metallicTexture.BindToLocation(3);
		pbrShader->setUniformFloat2("_metallicValue", Metallic, 0.0f);
	}
	else 
	{
		pbrShader->setUniformFloat2("_metallicValue", Metallic, 1.0f);
	}


	if (ambientOcclusionTexture.containsData())
	{
		ambientOcclusionTexture.BindToLocation(4);
		pbrShader->setUniformFloat("_ambientOcculusionValue", 0.0f);
	}
	else
	{
		pbrShader->setUniformFloat("_ambientOcculusionValue", 1.0f);
	}

	// height map location 5
	if (emissiveTexture.containsData())
	{
		emissiveTexture.BindToLocation(6);
		pbrShader->setUniformFloat4("_emissionColorValue", Emission.r, Emission.g, Emission.b,0.0f);
	}
	else
	{
		pbrShader->setUniformFloat4("_emissionColorValue", Emission.r, Emission.g, Emission.b, 1.0f);
	}


		pbrShader->setUniformInt("_albedoMap", 0);
		pbrShader->setUniformInt("_normalMap", 1);
		pbrShader->setUniformInt("_roughnessMap", 2);
		pbrShader->setUniformInt("_metallicMap", 3);
		pbrShader->setUniformInt("_ambientOcculusionMap", 4);
		pbrShader->setUniformInt("_emissionMap", 6);
	
	
	
	
}