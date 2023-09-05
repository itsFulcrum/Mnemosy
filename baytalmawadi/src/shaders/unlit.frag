#version 450 core


in vec4 position;
in vec4 normal;
in vec4 normalWS;
in vec2 uv;
in vec3 color;


uniform sampler2D colorMap;

out vec4 fragmentColor;



void main()
{
	vec4 color = vec4(1.0f,1.0f,1.0f,1.0f);
	fragmentColor = color;
}
