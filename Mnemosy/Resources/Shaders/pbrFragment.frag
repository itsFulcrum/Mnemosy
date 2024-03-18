#version 450 core

// includes
// using third party "HASHTAG"include parser
#include includes/lighting.glsl
#include includes/mathFunctions.glsl
#include includes/colorFunctions.glsl
#include includes/samplePbrMaps.glsl


// Vertex shader inputs
in vec2 uv;
in vec3 position_WS;
in vec3 normal_WS;
in mat3 tangentToWorldMatrix;

// Scene inputs
uniform vec3 _lightPositionWS;
uniform vec3 _lightDirectionWS;
uniform vec3 _lightColor;
uniform float _lightStrength;
uniform int _lightType; // 0 = directional, 1 = point
uniform float _lightAttentuation;
uniform vec3 _cameraPositionWS;
uniform float _skyboxRotation;
uniform float _skyboxExposure;



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
	////// SURFACE DATA ============================================================================================= ////
	//// ============================================================================================================ ////




			SurfaceData surfaceData;
			vec4 albedoAlpha = sampleAlbedoAlphaMap(_albedoMap,uv,_albedoColorValue);

			surfaceData.albedo = albedoAlpha.rgb;
		  surfaceData.normal = sampleNormalMap(_normalMap,uv,tangentToWorldMatrix,_normalValue);
		  surfaceData.emissive = sampleEmissionMap(_emissionMap,uv,_emissionColorValue);
		  surfaceData.emissionStrength = _emissionStrength;
		  surfaceData.roughness = sampleRoughnessMap(_roughnessMap,uv,_roughnessValue);
		  surfaceData.metallic = sampleMetallicMap(_metallicMap,uv,_metallicValue);
		  surfaceData.ambientOcclusion = sampleAmbientOcclusionMap(_ambientOcculusionMap,uv,_ambientOcculusionValue);
		  surfaceData.alpha = albedoAlpha.a;

	////// LIGHTTING DATA =========================================================================================== ////
	//// ============================================================================================================ ////
			LightingData lightingData;

			vec3 fragmentPosition  = position_WS;

			lightingData.fragmentPosition = fragmentPosition;
			lightingData.viewDirection = normalize(fragmentPosition - _cameraPositionWS);
			lightingData.skyboxRotation = _skyboxRotation;
			lightingData.skyboxExposure = _skyboxExposure;
			lightingData.lightPosition = _lightPositionWS;
			lightingData.lightColor = _lightColor * _lightStrength;
			lightingData.lightType = _lightType;
			lightingData.lightAttentuation = _lightAttentuation;

	////// SHADING ================================================================================================== ////
	//// ============================================================================================================ ////

			vec4 shadedFragmentColorLinear = lightingPBR(surfaceData,lightingData,_irradianceMap,_prefilterMap,_brdfLUT);

	////// POST PROCCESSING ========================================================================================= ////
	//// ============================================================================================================ ////
			//fragmentOutputColor	= vec4(0.0,0.0,0.0,1.0);

			fragmentOutputColor = postProcess(shadedFragmentColorLinear,0.0);

			//fragmentOutputColor	= vec4(0.0,0.0,0.0,1.0);
			//float r = texture(_roughnessMap,uv).r;
			//float r = sampleRoughnessMap(_roughnessMap,uv,_roughnessValue);
			//fragmentOutputColor.rgb = vec3(r,r,r);

			//fragmentOutputColor.rgb = texture(_brdfLUT, uv).rgb;

}
