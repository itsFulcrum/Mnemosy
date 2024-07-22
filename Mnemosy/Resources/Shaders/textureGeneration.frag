#version 450 core

// intput from vertex shader
in vec2 uv;

//outputs
out vec4 fragmentOutputColor;

// uniform Data
uniform int _mode;

uniform sampler2D _texture0;
// For future channel packing
//uniform sampler2D _texture1;
//uniform sampler2D _texture2;
//uniform sampler2D _texture3;

vec4 Mode0_FlipNormalYChannel(sampler2D normalMapSampler)
{
  vec4 normalMap = texture(normalMapSampler,uv);
  normalMap.y = 1 - normalMap.y;
  return vec4(normalMap.xyz,1.0f);
}

// for converting between roughness and smoothness textures
vec4 Mode1_InvertRoughness(sampler2D roughnessMapSampler){

  float roughness =  1 - texture(roughnessMapSampler,uv).r;
  return vec4(roughness,0.0f,0.0f,0.0f);
}

vec4 Mode2_OpacityFromAlbedoAlpha(sampler2D albedoSampler){

  float opacity = texture(albedoSampler,uv).a;
  return vec4(opacity,0.0f,0.0f,0.0f);
}

// Fragment Shader
void main()
{
  fragmentOutputColor = vec4(1.0f,0.0f,1.0f,1.0f);

  if(_mode == 0){
    fragmentOutputColor = Mode0_FlipNormalYChannel(_texture0);
  }
  else if(_mode == 1){
    fragmentOutputColor = Mode1_InvertRoughness(_texture0);
  }
  else if(_mode == 2){
    fragmentOutputColor = Mode2_OpacityFromAlbedoAlpha(_texture0);
  }


}
