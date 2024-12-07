#version 450 core

// vertex Data
layout (location=0) in vec3 aPos;

// uniform data
uniform mat4 _modelMatrix;
uniform mat4 _viewMatrix;
uniform mat4 _projectionMatrix;


void main()
{	
	gl_Position = _projectionMatrix * _viewMatrix * _modelMatrix * vec4(aPos.xyz, 1.0);

	// outputs to fragment shader
	//position_WS = vec3(modelMat * vec4(aPos.xyz, 1.0));
}
