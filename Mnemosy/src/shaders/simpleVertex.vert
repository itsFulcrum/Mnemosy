#version 450 core
layout (location=0) in vec3 aPos;
layout (location=1) in vec3 aColor;

out vec4 position;
out vec4 vertexColor;

void main()
{
	float scale = 1.0f;
	gl_Position = vec4(aPos.xyz, 1.0);

	// outputs to fragment shader
	position = vec4(aPos.xyz, 1.0);
	vertexColor = vec4(aColor.rgb,1.0);
}
