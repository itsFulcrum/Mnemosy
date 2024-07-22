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
//in vec3 cameraPositionWS;

in mat3 tangentToWorldMatrix;
in vec2 screenUV;
in vec2 pixelSize; // width and height of the framebufffer


in mat3 TBN;
//in vec3 fragPos_TS;
//in vec3 viewDir_TS;

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

uniform samplerCube _irradianceMap; // sampler 8
uniform samplerCube _prefilterMap;	// sampler 9
uniform sampler2D _brdfLUT; 				// sampler 10


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
uniform vec2 _normalValue; // x = _normalStrength y = indicate if normal is bound our not 0 = is bound, 1 = not bound
uniform bool _heightAssigned; // if Height map is assigned
uniform float _heightDepth;
uniform vec2 _opacityValue; // x = opacityThreshold, y = indicates if thexture is bound 0 = is bound 1 = not Bound

// adsitional settings
uniform float _emissionStrength;
uniform bool _useEmissiveMapAsMask;

//PBR Texture maps inputs
uniform sampler2D _albedoMap; // sampler  0
uniform sampler2D _normalMap; // sampler 1
uniform sampler2D _roughnessMap; // sampler 2
uniform sampler2D _metallicMap; // sampler 3
uniform sampler2D _ambientOcculusionMap; // sampler 4
uniform sampler2D _emissionMap; // sampler 5
uniform sampler2D _heightMap; // sampler 6
uniform sampler2D _opacityMap; // sampler 7


//outputs
out vec4 fragmentOutputColor;

void ApplyAlphaDiscard(float opacity,vec2 opacityValue,vec2 screenSpaceUV, vec2 framebufferSizeXY){

	if(opacityValue.y > 0.0005)
		return;

	float threshold = opacityValue.x * 2.0f;
	vec2 DitherCoordinate = screenSpaceUV  * (framebufferSizeXY);
	float dither = get_Bayer(int(DitherCoordinate.x),int(DitherCoordinate.y));
	float discardValue = step(dither*threshold,opacity);

		// discard pixels
	 if(discardValue <= 0.5f)
	 	discard;
}


vec2 SteepParralaxUV(vec2 uvs){

	float hightScale = _heightDepth * 0.01f;

	// working in tangent space
	vec3 viewPos_TS = TBN * _cameraPositionWS;
	vec3 fragPos_TS = TBN * position_WS;
	vec3 viewDir_TS = normalize(viewPos_TS - fragPos_TS);


	float minLayers = 2.0f  * _heightDepth + 1.0f;
	float maxLayers = 16.0f * _heightDepth;
	float numLayers = mix(maxLayers, minLayers, max(dot(vec3(0.0, 0.0, 1.0), viewDir_TS), 0.0));

	// the depth to sample for each layer
	float layerDepth = 1.0 / numLayers;

	float currentLayerDepth = 0.0f;

	vec2 offset = viewDir_TS.xy * hightScale / numLayers;

	// get initial values
	vec2  currentUV = uvs;
	float currentDepthMapValue = 1 - texture(_heightMap, currentUV).r;

	while(currentLayerDepth < currentDepthMapValue)
	{
	 currentUV -= offset;
	 currentDepthMapValue = 1- texture(_heightMap, currentUV).r;
	 currentLayerDepth += layerDepth;
	}

	return currentUV;
}


void main()
{
	////// SURFACE DATA ============================================================================================= ////
	//// ============================================================================================================ ////
	//fragmentOutputColor	= vec4(0.0,0.0,0.0,1.0);

		vec2 UVCorrds = uv;

		if(_heightAssigned){
			UVCorrds = SteepParralaxUV(uv);
		}

///// ===


		float opacity = SampleOpacityMap(_opacityMap,UVCorrds);
		ApplyAlphaDiscard(opacity,_opacityValue,screenUV,pixelSize);


			SurfaceData surfaceData;
			vec4 albedoAlpha = sampleAlbedoAlphaMap(_albedoMap,UVCorrds,_albedoColorValue);

			surfaceData.albedo = albedoAlpha.rgb;
		  surfaceData.normal = sampleNormalMap(_normalMap,UVCorrds,_normalValue.x,tangentToWorldMatrix,_normalValue.y);
		  surfaceData.emissive = sampleEmissionMap(_emissionMap,UVCorrds,_emissionColorValue,_useEmissiveMapAsMask);
		  surfaceData.emissionStrength = _emissionStrength;
		  surfaceData.roughness = sampleRoughnessMap(_roughnessMap,UVCorrds,_roughnessValue);
		  surfaceData.metallic = sampleMetallicMap(_metallicMap,UVCorrds,_metallicValue);
		  surfaceData.ambientOcclusion = sampleAmbientOcclusionMap(_ambientOcculusionMap,UVCorrds,_ambientOcculusionValue);
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
			//fragmentOutputColor.rgb = vec3(1,1,1);
			//fragmentOutputColor.rgb = texture(_brdfLUT, uv).rgb;

}
