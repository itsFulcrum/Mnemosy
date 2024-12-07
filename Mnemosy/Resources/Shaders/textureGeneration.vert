#version 450 core

// vertex Data
layout (location=0) in vec2 aPos;
layout (location=4) in vec2 aTexCoord0;

// output to fragment shader
out vec2 uv;

void main()
{
	gl_Position = vec4(aPos.xy, 0.0f, 1.0f);
  	uv = aTexCoord0;
}
