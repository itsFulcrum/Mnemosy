#version 450 core

#include includes/colorFunctions.glsl
#include includes/mathFunctions.glsl

precision highp float;


// vertex input
in vec3 cubeMapSampleVector;
in vec3 cubeSampleRight;
in vec3 cubeSampleUp;

//in vec2 screenSpacePos;
in vec4 fragPos;
//in vec4 fragPosNormalized;



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


  int maxLod = 8; // get max lod as uniform because it depends on skybox resolution.

  float sampleLOD =   saturate(_blurRadius) * float(maxLod);
  skyboxColor.rgb = textureLod(_skybox, cubeMapSampleVector, sampleLOD).rgb;




  skyboxColor.rgb = lerp(_skyboxColorValue.rgb , skyboxColor.rgb,_skyboxColorValue.w);


  skyboxColor = applyExposure(skyboxColor,_exposure);



  vec3 background = srgb_to_linear_cheap(vec4(_backgroundColor,1.0f)).rgb;


  

  vec4 fragPosNormalized = normalize(fragPos.xyzw);
  vec2 screenUV = fragPosNormalized.xy / fragPosNormalized.w;
  //vec2 screenUV = fragPos.xy / fragPos.w;


  float gradient = length(screenUV);

  vec3 gradientColor = lerp(background,vec3(0.0f,0.0f,0.0f),_gradientOpacity);

  background = lerp(background,gradientColor, gradient);

  fragmenColorLinear.rgb = mix(background.rgb,skyboxColor.rgb,_opacity);




  // float radi = RadicalInverse_VdC(pixelX * pixelY);
  // vec2 ham = Hammersley(pixelX,pixelY);
  // //fragmenColorLinear.rgb = vec3(radi);
  
  // //vec3 posNorm = normalize(fragPos.xyz);
  // fragmenColorLinear.rg = screenSpace01.xy;

  //fragmenColorLinear.rgb = vec3(simpleHash(screenUV));
  
  // POST PROCCESSING

// ==============  TEST BLOCK  blue noise

  // const int screenRes = 512;

  // vec2 screenUV01 = (screenUV.xy + 1.0f) * 0.5f;

  // int pixelX = int(screenUV01.x * float(screenRes));
  // int pixelY = int(screenUV01.y * float(screenRes));

  // //float radi = RadicalInverse_VdC(pixelX * pixelY);
  // //vec2 ham = Hammersley(pixelX,pixelY);

  // fragmentOutputColor = vec4(0.0f,0.0f,0.0f,1.0f);

  // float whiteNoise = WhiteNoise2DTo1D(screenUV01);


  // vec2 pixelSize = vec2( float(pixelX),  float(pixelY));

  // float blueNoise = Noise_BlueNoise(pixelSize);


  // //blueNoise = ReshapeUniformToTriangle(blueNoise);



  // float discardValue = step( blueNoise * _gradientOpacity, screenUV01.y);


  // fragmentOutputColor.rgb = vec3(discardValue);



// ==============  TEST BLOCK END

  fragmentOutputColor = postProcess(fragmenColorLinear,_postExposure);
}


