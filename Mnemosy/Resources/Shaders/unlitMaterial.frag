#version 450 core

// includes
// using third party "HASHTAG"include parser
#include includes/lighting.glsl
#include includes/mathFunctions.glsl
#include includes/colorFunctions.glsl
#include includes/samplePbrMaps.glsl


// Vertex shader inputs
//in float vertexAO;
in vec2 uv;
in vec3 position_WS;
in vec3 normal_WS;
//in vec3 cameraPositionWS;

in mat3 tangentToWorldMatrix;
in vec2 screenUV;
in vec2 pixelSize; // width and height of the framebufffer


in mat3 TBN;


//PBR Value inputs

uniform sampler2D _textureSampler; // sampler 0

uniform float _textureAssigned; // 0 if no texture 1 if texture 
uniform float _alphaThreshold;
uniform bool _useDitheredAlpha;
uniform bool _useAlpha;



//outputs
out vec4 fragmentOutputColor;

// void ApplyAlphaDiscard(float opacity,vec2 opacityValue,vec2 screenSpaceUV, vec2 framebufferSizeXY, bool useDithered){

// 	if(opacityValue.y > 0.0005) // return if no texture is bound
// 		return;


// 	float discardValue = 1.0f;
// 	float threshold = opacityValue.x;


// 	if(useDithered) {

// 		threshold *= threshold;
// 		vec2 DitherCoordinate = screenSpaceUV  * (framebufferSizeXY);
// 		float dither = get_Bayer(int(DitherCoordinate.x),int(DitherCoordinate.y));
// 		discardValue = step( dither*threshold, opacity);
// 	}
// 	else {
// 		discardValue = step(threshold,opacity);
// 	}

// 	// discard pixels
// 	if(discardValue <= 0.5f)
// 	 	discard;
// }

void main()
{
	////// SURFACE DATA ============================================================================================= ////
	//// ============================================================================================================ ////



		vec3 defaultColor = vec3(0.5f,0.5f,0.5f); 

		// TODo apply srgb 
		vec4 tex = texture(_textureSampler,uv).rgba;
		float opacity = tex.a;

		vec4 texLinear = srgb_to_linear(tex);

		vec3 color = lerp(defaultColor.rgb,texLinear.rgb,_textureAssigned);


	vec4 shadedFragmentColorLinear = vec4(color.rgb, opacity);

	// TODO hanld alpha clip

	// 	vec3 viewDir_WS = normalize(position_WS - _cameraPositionWS);

	// 	vec2 UVCorrds = SteepParralaxUV(_heightAssigned, _heightMap, uv, _heightDepth,_maxHeight,_cameraPositionWS,position_WS );

	// 	float opacity = SampleOpacityMap(_opacityMap,UVCorrds);
	// 	ApplyAlphaDiscard(opacity,_opacityValue,screenUV,pixelSize,_useDitheredAlpha);

	// 	SurfaceData surfaceData;
	// 	vec4 albedoAlpha = sampleAlbedoAlphaMap(_albedoMap,UVCorrds,_albedoColorValue);

	// 	surfaceData.albedo = albedoAlpha.rgb;
	// 	surfaceData.normal = sampleNormalMap(_normalMap,UVCorrds,_normalValue.x,tangentToWorldMatrix,_normalValue.y);
	// 	surfaceData.emissive = sampleEmissionMap(_emissionMap,UVCorrds,_emissionColorValue,_useEmissiveMapAsMask);
	// 	surfaceData.emissionStrength = _emissionStrength;
	// 	surfaceData.roughness = sampleRoughnessMap(_roughnessMap,UVCorrds,_roughnessValue);
	// 	surfaceData.metallic = sampleMetallicMap(_metallicMap,UVCorrds,_metallicValue);
	// 	surfaceData.ambientOcclusion = sampleAmbientOcclusionMap(_ambientOcculusionMap,UVCorrds,_ambientOcculusionValue) * vertexAO;
	// 	surfaceData.alpha = albedoAlpha.a;

	// ////// LIGHTTING DATA =========================================================================================== ////
	// //// ============================================================================================================ ////
	// 		LightingData lightingData;

	// 		lightingData.fragmentPosition = position_WS;
	// 		lightingData.viewDirection = viewDir_WS;
	// 		lightingData.skyboxRotation = _skyboxRotation;
	// 		lightingData.skyboxExposure = _skyboxExposure;
	// 		lightingData.lightPosition = _lightPositionWS;
	// 		lightingData.lightColor = _lightColor * _lightStrength;
	// 		lightingData.lightType = _lightType;
	// 		lightingData.lightAttentuation = _lightAttentuation;

	////// SHADING ================================================================================================== ////
	//// ============================================================================================================ ////

		//vec4 shadedFragmentColorLinear = lightingPBR(surfaceData,lightingData,_irradianceMap,_prefilterMap,_brdfLUT);


	////// POST PROCCESSING ========================================================================================= ////
	//// ============================================================================================================ ////
			fragmentOutputColor	= vec4(0.0,0.0,0.0,1.0);
			fragmentOutputColor = postProcess(shadedFragmentColorLinear,0.0);

}
