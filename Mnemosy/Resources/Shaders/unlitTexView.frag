#version 450 core

// includes
// using third party "HASHTAG"include parser
//include includes/lighting.glsl
#include includes/mathFunctions.glsl
#include includes/colorFunctions.glsl
#include includes/samplePbrMaps.glsl


// Vertex shader inputs
in vec2 uv;


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

uniform int _mode;
// MNSY_RENDERMODE_ALBEDO						= 1,
// MNSY_RENDERMODE_ROUGHNESS				= 2,
// MNSY_RENDERMODE_METALLIC					= 3,
// MNSY_RENDERMODE_NORMAL						= 4,
// MNSY_RENDERMODE_AMBIENTOCCLUSION	= 5,
// MNSY_RENDERMODE_EMISSION					= 6,
// MNSY_RENDERMODE_HEIGHT						= 7,
// MNSY_RENDERMODE_OPACITY					= 8
//outputs
out vec4 fragmentOutputColor;

void main()
{
			vec4 color = vec4(0.0f,0.0f,0.0f,0.0f);

			// depending on the mode we will render different texture
			// Albedo
			if(_mode == 1) {

				color.rgb = sampleAlbedoAlphaMap(_albedoMap,uv,_albedoColorValue).rgb;
			}

			// roughness
			else if (_mode == 2) {
				float rough = sampleRoughnessMap(_roughnessMap,uv,_roughnessValue);
				color.rgb = vec3(rough);
			}

			// metallic
			else if (_mode == 3) {
				float metal = sampleMetallicMap(_metallicMap,uv,_metallicValue);
				color.rgb = vec3(metal);
			}

			// normal
			else if (_mode == 4) {
				vec3 normalMap = texture(_normalMap,uv).xyz;
		    normalMap = lerp(normalMap.xyz, vec3(0.5f,0.5f,1.0f),_normalValue.y);
				color.rgb = normalMap;
			}

			// AO
			else if(_mode == 5) { // ao
				float ao = sampleAmbientOcclusionMap(_ambientOcculusionMap,uv,_ambientOcculusionValue);
				color.rgb = vec3(ao);
			}

			// Emissive
			else if(_mode == 6) {
				vec3 emit = sampleEmissionMap(_emissionMap,uv,_emissionColorValue,_useEmissiveMapAsMask);
				color.rgb = emit;
			}

			// Height
			else if(_mode == 7) {
				float h = 0.0f;
				if(_heightAssigned){
					h = texture(_heightMap,uv).r;
				}
				color.rgb = vec3(h);
			}

			// opacity
			else if(_mode == 8){

				float opacity = lerp(SampleOpacityMap(_opacityMap,uv), 1.0f,_opacityValue.y);
				color.rgb = vec3(opacity);
			}

	////// POST PROCCESSING ========================================================================================= ////
	//// ============================================================================================================ ////

			fragmentOutputColor	= vec4(1.0,0.0,0.0,1.0);
			fragmentOutputColor.rgb	= color.rgb;

			// For color textures albedo and emission we want to apply gamma correction at the end
			if(_mode == 1 || _mode == 6){
				fragmentOutputColor = postProcess(color,0.0);
			}

}
