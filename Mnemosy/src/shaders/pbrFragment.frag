#version 450 core
// using third party "HASHTAG"include parser
#include includes/mathFunctions.glsl
#include includes/pbrLightingTerms.glsl
#include includes/colorFunctions.glsl
#include includes/samplePbrMaps.glsl

// Vertex shader inputs
in vec2 uv;
in vec3 position_WS;
in vec3 normal_WS;
in mat3 tangentToWorldMatrix;

// Scene inputs
uniform vec3 _lightColor;
uniform float _lightStrength;
uniform vec3 _lightPositionWS;
uniform vec3 _cameraPositionWS;

uniform samplerCube _irradianceMap;
uniform samplerCube _prefilterMap;
uniform sampler2D _brdfLUT;

//PBR Value inputs
// I am using the last parameter of each value vector to
// lerp between the value and the sampled texture.
// this is done so that if no texture is bound to a specific slot the shader will use the value
// and if a texture is bound it will use the texture sample as the pbr value.
uniform vec4 _albedoColorValue;
uniform vec2 _roughnessValue;
uniform vec2 _metallicValue;
uniform vec4 _emissionColorValue;
uniform float _ambientOcculusionValue;
uniform float _normalValue;

uniform float _normalStrength;
uniform float _emissionStrength;

//PBR Texture maps inputs
uniform sampler2D _albedoMap;
uniform sampler2D _normalMap;
uniform sampler2D _roughnessMap;
uniform sampler2D _metallicMap;
uniform sampler2D _ambientOcculusionMap;
uniform sampler2D _emissionMap;

//outputs
out vec4 fragmentOutputColor;

void main()
{
	////// SETUP ==================================================================================================== ////
	//// ============================================================================================================ ////

			// set pbr values
			// if no albedo texture is bound alpha will probably be 0 which is not the nicest
			vec4 albedoAlphaMap = sampleAlbedoAlphaMap(_albedoMap,uv,_albedoColorValue);

			vec3 albedo = albedoAlphaMap.rgb;
			float alpha = albedoAlphaMap.a;
			float roughness = sampleRoughnessMap(_roughnessMap,uv,_roughnessValue);
			float metallic = sampleMetallicMap(_metallicMap,uv,_metallicValue);
			float ambientOcclusion = sampleAmbientOcclusionMap(_ambientOcculusionMap,uv,_ambientOcculusionValue);
			vec3 emission = sampleEmissionMap(_emissionMap,uv,_emissionColorValue);

			//vec3 outColor = vec3(0.0,0.0,0.0);
		  //outColor = lerp(emission,_emissionColorValue.rgb,_emissionColorValue.w);

			// Normal is in worldspace // everything is done is worldspace
			vec3 normal = sampleNormalMap(_normalMap,uv,tangentToWorldMatrix,_normalValue);

	////// LIGHTTING DATA =========================================================================================== ////
	//// ============================================================================================================ ////

			vec3 position  = position_WS;
			vec3 viewDirection = normalize(position - _cameraPositionWS);
			vec3 lightPosition = _lightPositionWS;
			// light direction only if light is a directional light otherwise we have to calulate it per lightsource in the light loop
			vec3 lightDirection = normalize(position - lightPosition);


			// Light values
			// setting light color here for now but should be set from app via uniform
			float lightStrength = 0.0f;
			vec3 lightColor = vec3(1.0f,1.0f,1.0f) * lightStrength;

	////// PRECOMPUDED TERMS ======================================================================================== ////
	//// ============================================================================================================ ////
		// precompude some terms here already for performace as they are used frequently throughout the shader

			vec3 F0 = mix(vec3(dialectricF0),albedo,metallic);
			float NdotV = max(dot(normal, -viewDirection), 0.0);

	//// DIRECT LIGHTING ============================================================================================ ////
	//// ============================================================================================================ ////

		vec3 directRadiance = vec3(0.0f); // commonly denoted as Lo
		// if more then one lightsource then
		// loop over all non directional lightsources
	 	directRadiance += CookTorranceSpecularBRDF(position, normal,NdotV, -viewDirection,F0, lightPosition, lightColor, albedo, metallic, roughness);


	////// INDIRECT LIGHTING ======================================================================================== ////
	//// ============================================================================================================ ////
		vec3 F = FresnelSchlickRoughness(NdotV,F0,roughness);
		vec3 kS = F;
		vec3 kD = 1.0 -kS;
		kD *= 1.0 - metallic;

		vec3 R = reflect(viewDirection,normal);

		// INDIRECT DIFFUSE
		vec3 irradiance = texture(_irradianceMap, normal).rgb;
		vec3 diffuse = irradiance * albedo;

		// INDIRECT SPECULAR
		const float MAX_REFLECTION_LOD = 4.0;
		vec3 prefilteredColor = textureLod(_prefilterMap, R,  roughness * MAX_REFLECTION_LOD).rgb;
		vec2 envBRDF  = texture(_brdfLUT, vec2(NdotV, roughness)).rg;
		vec3 specular = prefilteredColor * (F * envBRDF.x + envBRDF.y);

		vec3 indirectRadiance = (kD * diffuse + specular) * ambientOcclusion;

	////// Emissive ================================================================================================= ////
	//// ============================================================================================================ ////

		vec3 emissive = emission * _emissionStrength;

	////// COMBINED LIGHTING ======================================================================================== ////
	//// ============================================================================================================ ////


	vec4 pixelColorLinear =	vec4(emissive +  indirectRadiance + directRadiance,alpha);


	////// POST PROCCESSING ========================================================================================= ////
	//// ============================================================================================================ ////
	fragmentOutputColor = vec4(0.0,0.0,0.0,1.0);



	fragmentOutputColor = postProcess(pixelColorLinear,0.0f);
	//fragmentOutputColor.rgb = vec3(normal);

}