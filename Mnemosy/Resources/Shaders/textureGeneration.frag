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
uniform bool _channel_r_invert;
uniform float _channel_r_defaultValue;
// loaction 2
uniform sampler2D _channel_g;
uniform bool _channel_g_isAssigned;
uniform bool _channel_g_isSingleChannel;
uniform bool _channel_g_invert;
uniform float _channel_g_defaultValue;
// loaction 3
uniform sampler2D _channel_b;
uniform bool _channel_b_isAssigned;
uniform bool _channel_b_isSingleChannel;
uniform bool _channel_b_invert;
uniform float _channel_b_defaultValue;
// loaction 4
uniform sampler2D _channel_a;
uniform bool _channel_a_isAssigned;
uniform bool _channel_a_isSingleChannel;
uniform bool _channel_a_invert;
uniform float _channel_a_defaultValue;



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

  // ===== R - channel =====
  float R = 0.0f;
  if(_channel_r_isAssigned) {
    R = texture(_channel_r,uv).r;
  }
  else{
    R = _channel_r_defaultValue;
  }

  if(_channel_r_invert){
    R = 1.0f - clamp(R,0.0f,1.0f);
  }

  // ===== G - channel =====
  float G = 0.0f;

  if(_channel_g_isAssigned) {

    if(_channel_g_isSingleChannel) {
      G = texture(_channel_g,uv).r;
    }
    else {
      G = texture(_channel_g,uv).g;
    }
  }
  else {
    G = _channel_g_defaultValue;
  }

  if(_channel_g_invert){
    G = 1.0f - clamp(G,0.0f,1.0f);
  }

  // ===== B - channel =====
  float B = 0.0f;

  if(_channel_b_isAssigned) {

    if(_channel_b_isSingleChannel) {
      B = texture(_channel_b,uv).r;
    }
    else {
      B = texture(_channel_b,uv).b;
    }
  }
  else {
    B = _channel_b_defaultValue;
  }

  if(_channel_b_invert){
    B = 1.0f - clamp(B,0.0f,1.0f);
  }

  // ===== A - channel =====
  float A = 0.0f;

  if(_channel_a_isAssigned) {

    if(_channel_a_isSingleChannel) {
      A = texture(_channel_a,uv).r;
    }
    else {
      A = texture(_channel_a,uv).a;
    }
  }
  else {
    A = _channel_a_defaultValue;
  }

  if(_channel_a_invert){
    A = 1.0f - clamp(A,0.0f,1.0f);
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
    fragmentOutputColor = Mode3_CreateChannelPack();
  }


}
