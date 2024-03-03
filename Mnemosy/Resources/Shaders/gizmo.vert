#version 450 core

#include includes/mathFunctions.glsl

// vertex Data
layout (location=0) in vec3 aPos;
layout (location=4) in vec3 aColor;

// constants
float gizmoScale = 1.0f;
vec2 screenPosition = vec2(0.83,-0.85); // in normalised device coordinates

// uniform data
uniform mat4 _modelMatrix;
uniform mat4 _viewMatrix;
uniform mat4 _projectionMatrix;

// output data to fragment shader
out vec3 color;

void main()
{
	mat4 scaleMat = scale(gizmoScale,gizmoScale,gizmoScale);
	vec4 scaledPosition = scaleMat * vec4( aPos.xyz, 1.0);

	vec4 position = _projectionMatrix * _viewMatrix * _modelMatrix * vec4(scaledPosition.xyz, 1.0);

  // translate position in ndc
  mat4 translateMat = translate(screenPosition.x,screenPosition.y,0.0);
	position = translateMat * vec4(position.xyz, 1.0);
  position.w = 1;

	gl_Position = position;
	color = aColor;
}
