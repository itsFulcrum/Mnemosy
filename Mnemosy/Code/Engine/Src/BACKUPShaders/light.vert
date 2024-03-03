#version 450 core
// vertex Data
layout (location=0) in vec3 aPos;
//layout (location=1) in vec3 aNormal;
//layout (location=2) in vec3 aTangent;
//layout (location=3) in vec3 aBitangent;
//layout (location=4) in vec3 aColor;
//layout (location=5) in vec2 aTexCoord;

// uniform data
uniform mat4 _modelMatrix;
//uniform mat4 _normalMatrix;
uniform mat4 _viewMatrix;
uniform mat4 _projectionMatrix;


out vec3 position_WS;

void main()
{
	gl_Position = _projectionMatrix * _viewMatrix * _modelMatrix * vec4(aPos.xyz, 1.0);

	// outputs to fragment shader
	position_WS = vec3(_modelMatrix * vec4(aPos.xyz, 1.0));
}
