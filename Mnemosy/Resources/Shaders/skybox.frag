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

uniform samplerCube _skybox;
uniform samplerCube _irradianceMap;
uniform samplerCube _prefilterMap;

uniform float _exposure;
uniform vec3 _colorTint;

uniform float _blurRadius;
uniform int _blurSteps;
uniform vec3 _backgroundColor;
uniform float _opacity;
uniform float _gradientOpacity;


out vec4 fragmentOutputColor;

void main()
{
  vec4 fragmenColorLinear = vec4(0.0f,0.0f,0.0f,1.0f);
  vec4 skyboxColor = vec4(0.0f,0.0f,0.0f,1.0f);
  //skyboxColor = texture(_skybox, cubeMapSampleVector); // just normally sampling skybox

  // skybox blurring
  if(_blurSteps > 0 && _blurRadius > 0.0001f) {

    float increment = _blurRadius / _blurSteps * 0.1f;

    vec3 up = vec3(0.0f,1.0f,0.0f);
    //vec3 up = normalize(cubeSampleUp);
    vec3 right = normalize(cubeSampleRight);

    //float jx = simpleHash(cubeMapSampleVector.xy);
    for (int i = 0; i < _blurSteps; i++) {
      float rot = i * increment;

      //float jy = simpleHash(vec2(right.x,rot));
      //float jz = simpleHash(vec2(rot,right.y));
      //vec3 jitter = normalize(vec3(jx,jy,jx));
      //jitter *= 0.4f;
      //
      vec3 sampleUp = Rotate_About_Axis_Radians_float(cubeMapSampleVector,up,rot);
      skyboxColor += texture(_skybox, normalize(sampleUp));
      vec3 sampleDown = Rotate_About_Axis_Radians_float(cubeMapSampleVector,-up,rot);
      skyboxColor += texture(_skybox, normalize(sampleDown));
      vec3 sampleRight = Rotate_About_Axis_Radians_float(cubeMapSampleVector,right,rot);
      skyboxColor += texture(_skybox, normalize(sampleRight));
      vec3 sampleLeft = Rotate_About_Axis_Radians_float(cubeMapSampleVector,-right,rot);
      skyboxColor += texture(_skybox, normalize(sampleLeft));
      // diagonal
      vec3 sampleVec1 = Rotate_About_Axis_Radians_float(cubeMapSampleVector,up+right,rot);
      skyboxColor += texture(_skybox, normalize(sampleVec1));
      vec3 sampleVec2 = Rotate_About_Axis_Radians_float(cubeMapSampleVector,up-right,rot);
      skyboxColor += texture(_skybox, normalize(sampleVec2));
      vec3 sampleVec3 = Rotate_About_Axis_Radians_float(cubeMapSampleVector,-up+right,rot);
      skyboxColor += texture(_skybox, normalize(sampleVec3));
      vec3 sampleVec4 = Rotate_About_Axis_Radians_float(cubeMapSampleVector,-up-right,rot);
      skyboxColor += texture(_skybox, normalize(sampleVec4));

    }
    skyboxColor = skyboxColor / (_blurSteps * 8);

  }
  else { // no blurring
    skyboxColor = texture(_skybox, cubeMapSampleVector);
  }

  skyboxColor = applyExposure(skyboxColor,_exposure);

  skyboxColor.rgb *= _colorTint;
  vec3 background = srgb_to_linear_cheap(vec4(_backgroundColor,1.0f)).rgb;


  //vec2 screenSpace01 = (screenSpacePos + 1) *0.5;
  vec2 screenUV = fragPos.xy / fragPos.w;


  float gradient = saturate(length(screenUV));
  vec3 gradientColor = lerp(background,vec3(0.01f,0.01f,0.01f),_gradientOpacity);

  background = lerp(background,gradientColor, gradient);

  fragmenColorLinear.rgb = mix(background.rgb,skyboxColor.rgb,_opacity);

  // POST PROCCESSING
  //fragmentOutputColor = vec4(0.0f,0.0f,0.0f,1.0f);

  fragmentOutputColor = postProcess(fragmenColorLinear,0.0);
}


