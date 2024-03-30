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

// Fragment Shader
void main()
{
  fragmentOutputColor = vec4(0.0f,0.0f,0.0f,1.0f);

  if(_mode == 0){
    fragmentOutputColor = Mode0_FlipNormalYChannel(_texture0);
  }

}
