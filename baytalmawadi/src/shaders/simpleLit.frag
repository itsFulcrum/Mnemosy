#version 450 core
// using third party "HASHTAG"include parser
#include includes/mathFunctions.glsl

in vec2 uv;
in vec3 positionOS;
in vec3 positionWS;
in vec3 normalOS;
in vec3 normalWS;


uniform vec3 _lightColor;
uniform vec3 _lightStrength;
uniform vec3 _lightPositionWS;
uniform vec3 _cameraPositionWS;


uniform sampler2D colorMap;
uniform sampler2D textureMask;

vec3 lightDirection;
vec3 viewDirection;



void main()
{
	normalOS = normalize(normalOS);
	normalWS = normalize(normalWS);
	// if we use point light we would calc light direction here like this. if its a directional light we would set it via a uniform.
	lightDirection = normalize(positionWS - _lightPositionWS);
	viewDirection = normalize(positionWS -_cameraPositionWS);

// light attentuation
//	float lightDistance    = length(_lightPositionWS - positionWS);
//	float attenuation = 1.0 / (constantValue + linearValue * distance + quadraticValue * (distance * distance)); 

	vec4 colorTexture = texture(colorMap,uv);

	fragmentColor = colorTexture;
}
