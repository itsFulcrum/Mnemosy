#version 450 core

#include includes/colorFunctions.glsl
#include includes/mathFunctions.glsl

precision highp float;


// vertex input
in vec3 cubeMapSampleVector;
in vec3 cubeSampleRight;
in vec3 cubeSampleUp;
in vec2 screenSpacePos;

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
  fragmentOutputColor = vec4(1.0f,0.0f,1.0f,1.0f);
}
