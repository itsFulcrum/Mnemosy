#version 450 core

#include includes/colorFunctions.glsl

precision highp float;

out vec4 FragmentColor;

in vec3 cubeMapSampleVector;

uniform samplerCube _skybox;
uniform samplerCube _irradianceMap;
uniform samplerCube _prefilterMap;


uniform float _exposure;
uniform vec3 _colorTint;

void main()
{
    vec4 skyboxColor = texture(_skybox, cubeMapSampleVector);
    //vec4 irradianceColor = textureLod(_irradianceMap, cubeMapSampleVector,1);
    //vec4 prefilteredColor = texture(_prefilterMap, cubeMapSampleVector);

    skyboxColor = applyExposure(skyboxColor,_exposure);
    skyboxColor.rgb *= _colorTint;


    FragmentColor = skyboxColor;

    //FragmentColor.rgb = mix(skyboxColor.rgb, irradianceColor.rgb,0.85);

    FragmentColor = postProcess(FragmentColor,0.0);
}
