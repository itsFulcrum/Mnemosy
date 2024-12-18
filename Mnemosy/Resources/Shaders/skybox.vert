#version 450 core

#include includes/mathFunctions.glsl

layout (location = 0) in vec3 aPosition;


precision highp float;
out vec3 cubeMapSampleVector;
out vec3 cubeSampleRight;
out vec3 cubeSampleUp;
out vec4 fragPos;

uniform mat4 _viewMatrix;
uniform mat4 _projectionMatrix;
uniform float _rotation;
void main()
{
    cubeMapSampleVector = normalize( Rotate_About_Axis_Radians_float(aPosition, vec3(0.0,1.0,0.0), _rotation));
    cubeSampleRight = normalize(cross(cubeMapSampleVector,vec3(0.0,1.0,0.0)));
    cubeSampleUp = normalize(cross(cubeSampleRight,cubeMapSampleVector));

    fragPos = _projectionMatrix * _viewMatrix * vec4(aPosition,1.0f);

    gl_Position = fragPos.xyww;
}
