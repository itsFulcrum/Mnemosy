#version 450 core

// vertex Data
layout (location=0) in vec3 aPos;
layout (location=1) in vec3 aNormal;
layout (location=2) in vec3 aTangent;
// layout (location=3) in vec3 aColor;
layout (location=4) in vec2 aTexCoord;

// common uniform data
uniform mat4 _modelMatrix;
uniform mat4 _normalMatrix;
uniform mat4 _viewMatrix;
uniform mat4 _projectionMatrix;
uniform int _pixelWidth;
uniform int _pixelHeight;


uniform vec2 _uvTiling;

// output data to fragment shader
out vec2 uv;
out vec2 screenUV;
//out float vertexAO;
out vec3 position_WS;
out vec3 normal_WS;

out mat3 tangentToWorldMatrix;
out mat3 TBN;

out vec2 pixelSize; // width and height of framebuffer

void main()
{

	// apparently this is slightly faster than using the matrix directly because the shader compiler knows it can omit some instructions
	// see GodotCon - rendering talk 2024: https://www.youtube.com/watch?v=6ak1pmQXJbg
	mat4 modelMat  = mat4(_modelMatrix[0] ,_modelMatrix[1] ,_modelMatrix[2] ,vec4(0.0f,0.0f,0.0f,1.0f));
	mat4 normalMat = mat4(_normalMatrix[0],_normalMatrix[1],_normalMatrix[2],vec4(0.0f,0.0f,0.0f,1.0f));
	
	vec4 fragPos = _projectionMatrix * _viewMatrix * modelMat * vec4(aPos.xyz, 1.0);
	
	position_WS = vec3(modelMat * vec4(aPos.xyz, 1.0));
	normal_WS = vec4(normalMat * vec4(aNormal,0.0f)).xyz;

	// outputs to fragment shader
	//vertexAO = aColor.r; // ao is supposed to be baked into vertex color
	uv.xy = aTexCoord.xy * _uvTiling.xy;
	
	screenUV = fragPos.xy / fragPos.w * 0.5f + 0.5f;
	pixelSize = vec2(_pixelWidth,_pixelHeight);
	
	
	vec3 biTangent = cross(aNormal,aTangent);

	vec3 T = normalize(vec3(normalMat * vec4(aTangent,   0.0)));
  	vec3 B = normalize(vec3(normalMat * vec4(biTangent,  0.0)));
  	vec3 N = normalize(vec3(normalMat * vec4(aNormal,    0.0)));
  	
  	tangentToWorldMatrix = mat3(T, B, N);
	TBN = transpose(tangentToWorldMatrix);
	


	gl_Position = fragPos;
}
