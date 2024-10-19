#version 450 core

// vertex Data
layout (location=0) in vec3 aPos;
layout (location=1) in vec3 aNormal;
layout (location=2) in vec3 aTangent;
layout (location=3) in vec3 aBitangent;
layout (location=4) in vec3 aColor;
layout (location=5) in vec2 aTexCoord;

// uniform data
uniform mat4 _modelMatrix;
uniform mat4 _normalMatrix;
uniform mat4 _viewMatrix;
uniform mat4 _projectionMatrix;


uniform vec2 _uvTiling;
uniform int _pixelWidth;
uniform int _pixelHeight;

// output data to fragment shader
out vec2 uv;
out vec2 screenUV;
out float vertexAO;
out vec3 position_WS;
out vec3 normal_WS;

out mat3 tangentToWorldMatrix;
out mat3 TBN;

out vec2 pixelSize; // width and height of framebuffer



void main()
{
	vec4 fragPos = _projectionMatrix * _viewMatrix * _modelMatrix * vec4(aPos.xyz, 1.0);
	gl_Position = fragPos;

	// outputs to fragment shader
	vertexAO = aColor.r; // ao is supposed to be baked into vertex color
	uv = aTexCoord;
	uv.x = uv.x * _uvTiling.x;
	uv.y = uv.y * _uvTiling.y;
	position_WS = vec3(_modelMatrix * vec4(aPos.xyz, 1.0));
	normal_WS = vec4(_normalMatrix * vec4(aNormal,0.0f)).xyz;

	vec2 fragCoords = (fragPos.xy/ fragPos.w);
	screenUV = fragCoords * 0.5f + 0.5f;

	pixelSize = vec2(_pixelWidth,_pixelHeight);


	vec3 T = normalize(vec3(_normalMatrix * vec4(aTangent,   0.0)));
  	vec3 B = normalize(vec3(_normalMatrix * vec4(aBitangent, 0.0)));
  	vec3 N = normalize(vec3(_normalMatrix * vec4(aNormal,    0.0)));
  	tangentToWorldMatrix = mat3(T, B, N);

	//vec3 T1 = normalize(vec3(_modelMatrix * vec4(aTangent,   0.0)));
  	//vec3 B1 = normalize(vec3(_modelMatrix * vec4(aBitangent, 0.0)));
  	//vec3 N1 = normalize(vec3(_modelMatrix * vec4(aNormal,    0.0)));
	//TBN = transpose(mat3(T1, B1, N1));
	TBN = transpose(tangentToWorldMatrix);
}
