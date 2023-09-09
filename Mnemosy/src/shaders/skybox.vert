#version 450 core
layout (location = 0) in vec3 aPosition;
precision highp float;
out vec3 TexCoords;

uniform mat4 _viewMatrix;
uniform mat4 _projectionMatrix;

void main()
{
    TexCoords = aPosition;
    vec4 pos = _projectionMatrix * _viewMatrix * vec4(aPosition,1.0f);
    gl_Position = pos.xyww;
}
