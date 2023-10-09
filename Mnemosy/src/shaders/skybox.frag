#version 450 core
#include includes/colorFunctions.glsl

precision highp float;

out vec4 FragmentColor;

in vec3 cubeMapSampleVector;

uniform samplerCube _skybox;
uniform float _exposure;
uniform vec3 _tint;

void main()
{
    vec4 skyboxColor = texture(_skybox, cubeMapSampleVector);
    FragmentColor = skyboxColor * vec4(_tint,1.0);
    FragmentColor = postProcess(FragmentColor,0.0);
}
