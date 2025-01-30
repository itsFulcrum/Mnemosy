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


//uniform samplerCube _skybox;
uniform samplerCube _irradianceMap;
uniform samplerCube _prefilterMap;

uniform float _exposure;
uniform vec4  _skyboxColorValue; // rgb are skybox color, w indicates if samplers are bound where 1 = is bound and 0 = not bound

uniform vec3 _backgroundColor;
uniform float _opacity;
uniform float _gradientOpacity;
uniform float _blurRadius;
uniform int   _blurSteps;
uniform int _prefilterMaxMip;


out vec4 fragmentOutputColor;

void main()
{
  vec4 fragmenColorLinear = vec4(0.0f,0.0f,0.0f,1.0f);
  vec4 skyboxColor = vec4(0.0f,0.0f,0.0f,1.0f);


  int maxLod = _prefilterMaxMip -2; // get max lod as uniform because it depends on skybox resolution.

  // running this through a ease because prefilter drops exponentially so blurring with 0-1 range doesn't feel like linear blurr increase.
  float blurFactor = _blurRadius * _blurRadius;



  float sampleLOD = saturate(blurFactor) * float(maxLod);
  //skyboxColor.rgb = textureLod(_prefilterMap, cubeMapSampleVector, sampleLOD).rgb;





  int samples = int (25.0f * ( pow(-_blurRadius,3)+1.0f )  );

  if(_blurRadius < 0.001f){
    samples = 0;
  }

  // main sample has a weight of 1.
  vec3 skyColorBlur = textureLod(_prefilterMap, cubeMapSampleVector, sampleLOD).rgb;
  float totalWeight = 1.0f;


  vec3 cubeSampleUpRight = normalize(cubeSampleUp + cubeSampleRight);
  vec3 cubeSampleUpLeft = normalize(cubeSampleUp +  -cubeSampleRight);


  for(int i = 1; i < samples; i++){


    float weight = float(i) / float(samples);

    float offset = weight * blurFactor;

    vec3 north      = normalize(cubeMapSampleVector + (cubeSampleUp      * offset) );
    vec3 northEast  = normalize(cubeMapSampleVector + (cubeSampleUpRight * offset) );
    vec3 east       = normalize(cubeMapSampleVector + (cubeSampleRight  * offset) );
    vec3 southEast  = normalize(cubeMapSampleVector + (-cubeSampleUpLeft  * offset) );
    vec3 south      = normalize(cubeMapSampleVector + (-cubeSampleUp    * offset) );
    vec3 southWest  = normalize(cubeMapSampleVector + (-cubeSampleUpRight    * offset) );
    vec3 west       = normalize(cubeMapSampleVector + (-cubeSampleRight * offset) );
    vec3 northWest  = normalize(cubeMapSampleVector + (cubeSampleUpLeft * offset) );

    weight = 1.0f - weight;

    skyColorBlur += textureLod(_prefilterMap, north, sampleLOD).rgb * weight;
    skyColorBlur += textureLod(_prefilterMap, northEast, sampleLOD).rgb * weight;
    skyColorBlur += textureLod(_prefilterMap, east, sampleLOD).rgb  * weight;
    skyColorBlur += textureLod(_prefilterMap, southEast, sampleLOD).rgb  * weight;
    skyColorBlur += textureLod(_prefilterMap, south, sampleLOD).rgb * weight;
    skyColorBlur += textureLod(_prefilterMap, southWest, sampleLOD).rgb  * weight;
    skyColorBlur += textureLod(_prefilterMap, west, sampleLOD).rgb  * weight;
    skyColorBlur += textureLod(_prefilterMap, northWest, sampleLOD).rgb  * weight;

    totalWeight += (weight * 8);
  }



  skyboxColor.rgb = skyColorBlur.rgb  / totalWeight;


 //skyboxColor.rgb = textureLod(_prefilterMap, cubeMapSampleVector, sampleLOD).rgb;





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


