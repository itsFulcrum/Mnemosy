#version 450 core

// vertex Data
layout (location=0) in vec3 aPos;

// uniform data
uniform mat4 _modelMatrix;
uniform mat4 _viewMatrix;
uniform mat4 _projectionMatrix;


out vec3 position_WS;

void main()
{

	mat4 modelMat  = mat4(_modelMatrix[0] ,_modelMatrix[1] ,_modelMatrix[2] ,vec4(0.0f,0.0f,0.0f,1.0f));
	
	gl_Position = _projectionMatrix * _viewMatrix * modelMat * vec4(aPos.xyz, 1.0);

	// outputs to fragment shader
	position_WS = vec3(modelMat * vec4(aPos.xyz, 1.0));
}
