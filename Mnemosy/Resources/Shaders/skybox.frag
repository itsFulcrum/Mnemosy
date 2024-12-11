#version 450 core

#include includes/colorFunctions.glsl
#include includes/mathFunctions.glsl

precision highp float;


// vertex input
in vec3 cubeMapSampleVector;
in vec3 cubeSampleRight;
in vec3 cubeSampleUp;

in vec2 screenSpacePos;
in vec4 fragPos;



// uniforms

// gloabls
uniform float _postExposure;


uniform samplerCube _skybox;
uniform samplerCube _irradianceMap;
uniform samplerCube _prefilterMap;

uniform float _exposure;
uniform vec4  _skyboxColorValue; // rgb are skybox color, w indicates if samplers are bound where 1 = is bound and 0 = not bound

uniform vec3 _backgroundColor;
uniform float _opacity;
uniform float _gradientOpacity;
uniform float _blurRadius;
uniform int   _blurSteps;



out vec4 fragmentOutputColor;

void main()
{
  vec4 fragmenColorLinear = vec4(0.0f,0.0f,0.0f,1.0f);
  vec4 skyboxColor = vec4(0.0f,0.0f,0.0f,1.0f);


  int maxLod = 8;

  float sampleLOD =   saturate(_blurRadius * 0.5f) * float(maxLod);
  skyboxColor.rgb = textureLod(_skybox, cubeMapSampleVector, sampleLOD).rgb;




  skyboxColor.rgb = lerp(_skyboxColorValue.rgb , skyboxColor.rgb,_skyboxColorValue.w);


  skyboxColor = applyExposure(skyboxColor,_exposure);



  vec3 background = srgb_to_linear_cheap(vec4(_backgroundColor,1.0f)).rgb;


  //vec2 screenSpace01 = (screenSpacePos + 1) *0.5;
  vec2 screenUV = fragPos.xy / fragPos.w;


  float gradient = saturate(length(screenUV));
  vec3 gradientColor = lerp(background,vec3(0.01f,0.01f,0.01f),_gradientOpacity);

  background = lerp(background,gradientColor, gradient);

  fragmenColorLinear.rgb = mix(background.rgb,skyboxColor.rgb,_opacity);

  // POST PROCCESSING
  //fragmentOutputColor = vec4(0.0f,0.0f,0.0f,1.0f);

  fragmentOutputColor = postProcess(fragmenColorLinear,_postExposure);
}


