#ifndef MATH_FUNCTIONS_GLSL
#define MATH_FUNCTIONS_GLSL

float saturate(float value)
{
  return clamp(value,0.0f,1.0f);
}

float clamp01(float value)
{
  return clamp(value,0.0f,1.0f);
}

float oneMinus(float value)
{
    return 1.0f-value;
}

float lerp(float a, float b, float t)
{
  return (1.0f - t) * a + b * t;
}
float inverseLerp(float a, float b, float v)
{
  return (v-a) / (b - a);
}
float remap(float iMin,float iMax,float oMin,float oMax, float v)
{
  float t = inverseLerp(iMin,iMax,v);
  return lerp(oMin,oMax,t);
}
vec3 lerp(vec3 a,vec3 b, float t)
{
    vec3 result = vec3(0.0f,0.0f,0.0f);
    result.x = lerp(a.x,b.x,t);
    result.y = lerp(a.y,b.y,t);
    result.z = lerp(a.z,b.z,t);

    return result;
}
vec4 lerp(vec4 a,vec4 b, float t)
{
    vec4 result = vec4(0.0f,0.0f,0.0f,0.0f);
    result.x = lerp(a.x,b.x,t);
    result.y = lerp(a.y,b.y,t);
    result.z = lerp(a.z,b.z,t);
    result.w = lerp(a.w,b.w,t);

    return result;
}

// rotate vector around an axis (by unity)

vec3 Rotate_About_Axis_Radians_float(vec3 In, vec3 Axis, float Rotation)
{
    float s = sin(Rotation);
    float c = cos(Rotation);
    float one_minus_c = 1.0 - c;

    Axis = normalize(Axis);

    mat3x3 rot_mat;

/*
one_minus_c * Axis.x * Axis.x + c,            one_minus_c * Axis.x * Axis.y - Axis.z * s,     one_minus_c * Axis.z * Axis.x + Axis.y * s,
one_minus_c * Axis.x * Axis.y + Axis.z * s,   one_minus_c * Axis.y * Axis.y + c,              one_minus_c * Axis.y * Axis.z - Axis.x * s,
one_minus_c * Axis.z * Axis.x - Axis.y * s,   one_minus_c * Axis.y * Axis.z + Axis.x * s,     one_minus_c * Axis.z * Axis.z + c
*/

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

#endif
