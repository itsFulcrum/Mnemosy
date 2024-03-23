#ifndef MATH_FUNCTIONS_GLSL
#define MATH_FUNCTIONS_GLSL

precision highp float;

const float PI      = 3.14159265359;
const float PI_2    = 1.57079632679;
const float PI_4    = 0.78539816339;
const float INV_PI  = 0.31830988618;
float PHI =  1.61803398875; // = (1.0+sqrt(5.0))/2.0


// saturate
float saturate(float value)
{
  return clamp(value,0.0f,1.0f);
}
vec2 saturate(vec2 value)
{
  vec2 result = vec2(0.0f,0.0f);
  result.x = clamp(value.x,0.0f,1.0f);
  result.y = clamp(value.y,0.0f,1.0f);
  return result;
}
vec3 saturate(vec3 value)
{
  vec3 result = vec3(0.0f,0.0f,0.0f);
  result.x = clamp(value.x,0.0f,1.0f);
  result.y = clamp(value.y,0.0f,1.0f);
  result.z = clamp(value.z,0.0f,1.0f);
  return result;
}
vec4 saturate(vec4 value)
{
  vec4 result = vec4(0.0f,0.0f,0.0f,0.0f);
  result.x = clamp(value.x,0.0f,1.0f);
  result.y = clamp(value.y,0.0f,1.0f);
  result.z = clamp(value.z,0.0f,1.0f);
  result.w = clamp(value.w,0.0f,1.0f);
  return result;
}
// lerp
float lerp(float a, float b, float t)
{
  return (1.0f - t) * a + b * t;
}
vec2 lerp(vec2 a, vec2 b, float t)
{
  vec2 result = vec2(0.0f,0.0f);
  result.x = lerp(a.x,b.x,t);
  result.y = lerp(a.y,b.y,t);
  return result;
}
vec3 lerp(vec3 a,vec3 b, float t)
{
    vec3 result = vec3(0.0f,0.0f,0.0f);
    result.xy = lerp(a.xy,b.xy,t);
    result.z = lerp(a.z,b.z,t);
    return result;
}
vec4 lerp(vec4 a,vec4 b, float t)
{
    vec4 result = vec4(0.0f,0.0f,0.0f,0.0f);
    result.xyz = lerp(a.xyz,b.xyz,t);
    result.w = lerp(a.w,b.w,t);
    return result;
}
// inverse lerp
float inverseLerp(float a, float b, float v)
{
  return (v-a) / (b - a);
}
vec2 inverseLerp(vec2 a, vec2 b, float v)
{
  vec2 result = vec2(0.0f,0.0f);
  result.x = (v-a.x) / (b.x - a.x);
  result.y = (v-a.y) / (b.y - a.y);
  return result;
}
vec3 inverseLerp(vec3 a, vec3 b, float v)
{
  vec3 result = vec3(0.0f,0.0f,0.0f);
  result.x = (v-a.x) / (b.x - a.x);
  result.y = (v-a.y) / (b.y - a.y);
  result.z = (v-a.z) / (b.z - a.z);
  return result;
}
vec4 inverseLerp(vec4 a, vec4 b, float v)
{
  vec4 result = vec4(0.0f,0.0f,0.0f,0.0f);
  result.x = (v-a.x) / (b.x - a.x);
  result.y = (v-a.y) / (b.y - a.y);
  result.z = (v-a.z) / (b.z - a.z);
  result.w = (v-a.w) / (b.w - a.w);
  return result;
}

float remap(float iMin,float iMax,float oMin,float oMax, float v)
{
  float t = inverseLerp(iMin,iMax,v);
  return lerp(oMin,oMax,t);
}

// matrix stuff
mat4 scale(float x, float y, float z){
    return mat4(
        vec4(x,   0.0, 0.0, 0.0),
        vec4(0.0, y,   0.0, 0.0),
        vec4(0.0, 0.0, z,   0.0),
        vec4(0.0, 0.0, 0.0, 1.0)
    );
}

mat4 translate(float x, float y, float z){
    return mat4(
        vec4(1.0, 0.0, 0.0, 0.0),
        vec4(0.0, 1.0, 0.0, 0.0),
        vec4(0.0, 0.0, 1.0, 0.0),
        vec4(x,   y,   z,   1.0)
    );
}

mat4 RotateX(float phi){
    return mat4(
        vec4(1.,0.,0.,0),
        vec4(0.,cos(phi),-sin(phi),0.),
        vec4(0.,sin(phi),cos(phi),0.),
        vec4(0.,0.,0.,1.));
}

mat4 RotateY(float theta){
    return mat4(
        vec4(cos(theta),0.,-sin(theta),0),
        vec4(0.,1.,0.,0.),
        vec4(sin(theta),0.,cos(theta),0.),
        vec4(0.,0.,0.,1.));
}

mat4 RotateZ(float psi){
    return mat4(
        vec4(cos(psi),-sin(psi),0.,0),
        vec4(sin(psi),cos(psi),0.,0.),
        vec4(0.,0.,1.,0.),
        vec4(0.,0.,0.,1.));
}

// rotate vector around an axis (by unity)
vec3 Rotate_About_Axis_Radians_float(vec3 In, vec3 Axis, float Rotation)
{
    float s = sin(Rotation);
    float c = cos(Rotation);
    float one_minus_c = 1.0 - c;
    Axis = normalize(Axis);

    mat3x3 rot_mat; // check if declaration like mat3 rot_mat;  would actually complie as linter-glsl marks this as error but it works fine.
    rot_mat[0][0] = one_minus_c * Axis.x * Axis.x + c;
    rot_mat[1][0] = one_minus_c * Axis.x * Axis.y - Axis.z * s;
    rot_mat[2][0] = one_minus_c * Axis.z * Axis.x + Axis.y * s;
    rot_mat[0][1] = one_minus_c * Axis.x * Axis.y + Axis.z * s;
    rot_mat[1][1] = one_minus_c * Axis.y * Axis.y + c;
    rot_mat[2][1] = one_minus_c * Axis.y * Axis.z - Axis.x * s;
    rot_mat[0][2] = one_minus_c * Axis.z * Axis.x - Axis.y * s;
    rot_mat[1][2] = one_minus_c * Axis.y * Axis.z + Axis.x * s;
    rot_mat[2][2] = one_minus_c * Axis.z * Axis.z + c;
    return rot_mat * In;
}

// random functions
// bit shifting is not supported on all drivers but anything newer should be fine / webGL and GLES 2.0 wont work for example
float RadicalInverse_VdC(uint bits)
{
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}
// ----------------------------------------------------------------------------
vec2 Hammersley(uint i, uint N)
{
    return vec2(float(i)/float(N), RadicalInverse_VdC(i));
}

float simpleHash(vec2 uv)
{
    return fract(sin(7.289 * uv.x + 11.23 * uv.y) * 23758.5453);
}

#endif
