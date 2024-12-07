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

void ApplyAlphaDiscard(float opacity,float alphaThreshold,vec2 screenSpaceUV, vec2 framebufferSizeXY, bool useDithered){

	float discardValue = 1.0f;
	float threshold = alphaThreshold;


	if(useDithered) {

		threshold *= threshold;
		vec2 DitherCoordinate = screenSpaceUV  * (framebufferSizeXY);
		float dither = get_Bayer(int(DitherCoordinate.x),int(DitherCoordinate.y));
		discardValue = step( dither*threshold, opacity);
	}
	else {
		discardValue = step(threshold,opacity);
	}

	// discard pixels
	if(discardValue <= 0.5f)
	 	discard;
}

void main()
{
	////// SURFACE DATA ============================================================================================= ////
	//// ============================================================================================================ ////



	vec3 defaultColor = vec3(0.5f,0.5f,0.5f);
	vec4 tex = texture(_textureSampler,uv).rgba;

	float opacity = tex.a;

	

	vec4 texLinear = srgb_to_linear(tex);

	vec3 color = lerp(defaultColor.rgb,texLinear.rgb,_textureAssigned);


	vec4 shadedFragmentColorLinear = vec4(color.rgb, opacity);

	if(_useAlpha){
		ApplyAlphaDiscard(opacity,_alphaThreshold,screenUV,pixelSize,_useDitheredAlpha);
	
		shadedFragmentColorLinear.rgb *= opacity; // premultiplied alpha
	}

	////// POST PROCCESSING ========================================================================================= ////
	//// ============================================================================================================ ////
			fragmentOutputColor	= vec4(0.0,0.0,0.0,1.0);
			fragmentOutputColor = postProcess(shadedFragmentColorLinear,0.0);

}
