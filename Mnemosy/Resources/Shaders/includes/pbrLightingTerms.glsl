#ifndef PBR_LIGHTING_TERMS_GLSL
#define PBR_LIGHTING_TERMS_GLSL

const float PBR_LIGHTING_PI = 3.14159265359;
const float DIALECTRIC_F0 = 0.04f;

vec3 ImportanceSampleGGX(vec2 Xi, vec3 N, float roughness)
{
    float a = roughness*roughness;

    float phi = 2.0 * PBR_LIGHTING_PI * Xi.x;
    float cosTheta = sqrt((1.0 - Xi.y) / (1.0 + (a*a - 1.0) * Xi.y));
    float sinTheta = sqrt(1.0 - cosTheta*cosTheta);

    // from spherical coordinates to cartesian coordinates
    vec3 H;
    H.x = cos(phi) * sinTheta;
    H.y = sin(phi) * sinTheta;
    H.z = cosTheta;

    // from tangent-space vector to world-space sample vector
    vec3 up        = abs(N.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
    vec3 tangent   = normalize(cross(up, N));
    vec3 bitangent = cross(N, tangent);

    vec3 sampleVec = tangent * H.x + bitangent * H.y + N * H.z;
    return normalize(sampleVec);
    
}

// normal distribution function TrowbridgeReitzGGX
float NormalDistributionGGX(float NdotH, float roughness)
{
  // disney and epic concluded that squaring roughness looks more acurate
  float a = roughness*roughness;
  float a2 = a*a;
  //float NdotH = max(dot(N,H),0.0f);
  float NdotH2 = NdotH * NdotH;

  float num = a2;
  float denom = (NdotH * (a2 - 1.0f) + 1.0f);
  denom = PBR_LIGHTING_PI * denom * denom;
  return num / denom;
}

// geometry
float GeometrySchlickGGX(float NdotV, float k)
{
  // outcommented because I moved it into the GeometrySmith function
  //float r = (roughness +1.0f);
  //float k = (r*r) / 8.0f;
  float nom   = NdotV;
  float denom = NdotV * (1.0 - k) + k;

  return nom / denom;
}

float GeometrySmith(float NdotV, float NdotL, float roughness)
{
    // there is no need to calculate k twice in the GeometrySchlickGGX function
    float r = (roughness +1.0f);
    float k = (r*r) / 8.0f;

    float ggx1 = GeometrySchlickGGX(NdotV, k);
    float ggx2 = GeometrySchlickGGX(NdotL, k);

    return ggx1 * ggx2;
}

vec3 FresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0f - F0) * pow(clamp(1.0f - cosTheta,0.0f,1.0f),5.0f);
}

// for indirect diffuse light we dont have a micro-surface halfway that is influenced by roughness.
// we can inject roughness into the fresnel to simulate the effect as described by Sébastien Lagarde
vec3 FresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}



#endif
