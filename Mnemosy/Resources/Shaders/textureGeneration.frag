#version 450 core

// intput from vertex shader
in vec2 uv;

//outputs
out vec4 fragmentOutputColor;

// uniform Data
uniform int _mode;

uniform sampler2D _texture0;

// loaction 1
uniform sampler2D _channel_r;
uniform bool _channel_r_isAssigned;
uniform bool _channel_r_isSingleChannel;
// loaction 2
uniform sampler2D _channel_g;
uniform bool _channel_g_isAssigned;
uniform bool _channel_g_isSingleChannel;
// loaction 3
uniform sampler2D _channel_b;
uniform bool _channel_b_isAssigned;
uniform bool _channel_b_isSingleChannel;
// loaction 4
uniform sampler2D _channel_a;
uniform bool _channel_a_isAssigned;
uniform bool _channel_a_isSingleChannel;


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

vec4 Mode3_CreateChannelPack() {
  // R - channel
  float R = 0.0f;
  if(_channel_r_isAssigned) {
    R = texture(_channel_r,uv).r;
  }

  // G - channel
  float G = 0.0f;

  if(_channel_g_isAssigned) {

    if(_channel_g_isSingleChannel) {
      G = texture(_channel_g,uv).r;
    }
    else {
      G = texture(_channel_g,uv).g;
    }
  }

  // B - channel
  float B = 0.0f;

  if(_channel_b_isAssigned) {

    if(_channel_b_isSingleChannel) {
      B = texture(_channel_b,uv).r;
    }
    else {
      B = texture(_channel_b,uv).b;
    }
  }

  // A - channel
  float A = 0.0f;

  if(_channel_a_isAssigned) {

    if(_channel_a_isSingleChannel) {
      A = texture(_channel_a,uv).r;
    }
    else {
      A = texture(_channel_a,uv).a;
    }
  }

  return vec4(R,G,B,A);
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
  else if(_mode == 3){
    // create channel packed texture
    fragmentOutputColor = Mode3_CreateChannelPack();
  }


}
