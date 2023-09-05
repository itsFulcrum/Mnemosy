#version 450 core
layout (location=0) in vec3 aPos;
layout (location=1) in vec3 aNormal;
layout (location=2) in vec3 aColor;
layout (location=3) in vec2 aTexCoord;

uniform mat4 _modelMatrix;
uniform mat4 _normalMatrix;
uniform mat4 _viewMatrix;
uniform mat4 _projectionMatrix;


out vec4 position;
out vec4 normal;
out vec4 normalWS;
out vec2 uv;
out vec3 color;

void main()
{
	gl_Position = _projectionMatrix * _viewMatrix * _modelMatrix * vec4(aPos.xyz, 1.0);

	// outputs to fragment shader
	color = aColor;
	position = vec4(aPos.xyz, 1.0);
	normal = vec4(aNormal.xyz,1.0);
	normalWS = _modelMatrix * normal;
	uv = aTexCoord;
}
