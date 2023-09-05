#version 450 core
#include includes/colorFunctions.glsl
precision highp float;

out vec4 FragmentColor;

in vec3 TexCoords;

uniform samplerCube _skybox;
uniform float _exposure;

void main()
{

    //vec4 skyboxColor = textureLod(_skybox, TexCoords,0);
    vec4 skyboxColor = texture(_skybox, TexCoords);

    FragmentColor = skyboxColor;

    FragmentColor = postProcess(FragmentColor,0.0);
}
