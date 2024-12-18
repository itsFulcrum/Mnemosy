#ifndef MATH_FUNCTIONS_GLSL
#define MATH_FUNCTIONS_GLSL

precision highp float;

const float PI      = 3.14159265359;
const float ONE_OVER_PI = 0.31830988618;
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

vec3 NormalReconstructZ(vec2 In)
{
    float reconstructZ = sqrt(1.0 - saturate(dot(In.xy, In.xy)));
    vec3 normalVector = vec3(In.x, In.y, reconstructZ);
    return normalize(normalVector);
}
// this scales normal between 0 and 1 // this is honestly weird implmentation
vec3 NormalStrength(vec3 In, float Strength)
{
  return vec3(In.xy * Strength, lerp(1, In.z, saturate(Strength)));
}
vec3 NormalStrengthSpherical(vec3 n, float strength)
{
  // convert to cartesian to spherical cooridnates
  float theta = atan(n.y,n.x);
  float phi = atan(sqrt(n.x*n.x + n.y*n.y),n.z);

  phi = clamp(phi * strength,0,PI*0.5);


  // convert back spherical to cartesian coordinates
  vec3 z = vec3(0.0f,0.0f,1.0f);
  z = Rotate_About_Axis_Radians_float(z, vec3(0.0f,1.0f,0.0f), phi);
  z = Rotate_About_Axis_Radians_float(z, vec3(0.0f,0.0f,1.0f), theta);
  // this version doesnt work idk
  //vec4 z = vec4(0.0f,0.0f,1.0f,1.0f);
  //z = RotateY(theta) * z;
  //z = RotateZ(phi) * z;

  vec3 normalOut = z.xyz;
  return normalOut;
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

uint ReverseBits(uint x) {
    x = ((x & 0xaaaaaaaau) >> 1) | ((x & 0x55555555u) << 1);
    x = ((x & 0xccccccccu) >> 2) | ((x & 0x33333333u) << 2);
    x = ((x & 0xf0f0f0f0u) >> 4) | ((x & 0x0f0f0f0fu) << 4);
    x = ((x & 0xff00ff00u) >> 8) | ((x & 0x00ff00ffu) << 8);
    return (x >> 16) | (x << 16);
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


const int ditherBayer[8][8] =
{
{ 0, 32,  8, 40,  2, 34, 10, 42},
{48, 16, 56, 24, 50, 18, 58, 26},
{12, 44,  4, 36, 14, 46,  6, 38},
{60, 28, 52, 20, 62, 30, 54, 22},
{ 3, 35, 11, 43,  1, 33,  9, 41},
{51, 19, 59, 27, 49, 17, 57, 25},
{15, 47,  7, 39, 13, 45,  5, 37},
{63, 31, 55, 23, 61, 29, 53, 21}
};

float get_Bayer(int x, int y)
{
    float b = (ditherBayer[x%8][y%8] + 1 ) / 64.0f;
    return b;
}


float WhiteNoise3DTo1D(vec3 vec){

    vec3 smallValue = vec3(sin(vec.x),sin(vec.y),sin(vec.z));

    float random = dot(smallValue, vec3(12.9898, 78.233, 37.719));
    
    random = fract(sin(random) * 143758.5453);
    return random;
}


float WhiteNoise2DTo1D(vec2 vec){

    vec3 smallValue = vec3( sin(vec.x) , sin(vec.y) , cos(vec.x * vec.y * 37.719 + 11.23) );

    float random = dot(smallValue, vec3(12.9898, 78.233, 37.719));
    
    random = fract(sin(random) * 143758.5453);
    return random;
}



uint HilbertIndex(uvec2 p) {
    uint i = 0u;
    for(uint l = 0x4000u; l > 0u; l >>= 1u) {
        uvec2 r = min(p & l, 1u);
        
        i = (i << 2u) | ((r.x * 3u) ^ r.y);       
        p = r.y == 0u ? (0x7FFFu * r.x) ^ p.yx : p;
    }
    return i;
}


uint OwenHash(uint x, uint seed) { // seed is any random number
    x ^= x * 0x3d20adeau;
    x += seed;
    x *= (seed >> 16) | 1u;
    x ^= x * 0x05526c56u;
    x ^= x * 0x53a22864u;
    return x;
}




float Noise_BlueNoise(vec2 uvCoords){

    uint m = HilbertIndex(uvec2(uvCoords));
    m = OwenHash(ReverseBits(m), 0xe7843fbfu);
    m = OwenHash(ReverseBits(m), 0x8d8fb1e0u);
    //mask = float(ReverseBits(m)) / 4294967296.0;
    return float(ReverseBits(m)) / 4294967296.0;

}

float ReshapeUniformToTriangle(float v) {
    v = v * 2.0 - 1.0;
    v = sign(v) * (1.0 - sqrt(max(0.0, 1.0 - abs(v)))); // [-1, 1], max prevents NaNs
    return v + 0.5; // [-0.5, 1.5]
}


#endif
