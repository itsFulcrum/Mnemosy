#ifndef SAMPLE_PBR_MAPS
#define SAMPLE_PBR_MAPS

// ==== > REQURES colorFunctions.glsl to be included first

vec4 sampleAlbedoAlphaMap(sampler2D albedoSampler, vec2 uv)
{
  vec4 albedoMap = texture(albedoSampler,uv);
  albedoMap = clamp(albedoMap,0.0,1.0);
  return sRGBToLinear (albedoMap);
}
vec4 sampleEmissionMap(sampler2D emissionSampler, vec2 uv)
{
  vec4 emissionMap = texture(emissionSampler,uv);
  emissionMap = clamp(emissionMap,0.0,1.0);
  return sRGBToLinear (emissionMap);
}

float sampleRoughnessMap(sampler2D roughnessSampler,vec2 uv)
{
  vec4 roughnessMap = texture(roughnessSampler,uv);
  float roughness = desaturateLinear(roughnessMap.rgb);
  return clamp(roughness,0.0,1.0);
}
float sampleMetallicMap(sampler2D metallicSampler,vec2 uv)
{
  vec4 metallicMap = texture(metallicSampler,uv);
  float metallic = desaturateLinear(metallicMap.rgb);
  return clamp(metallic,0.0,1.0);
}

float sampleAmbientOcclusionMap(sampler2D ambientOcclusionSampler,vec2 uv)
{
  vec4 ambientOcclusionMap = texture(ambientOcclusionSampler,uv);
  float ao = desaturateLinear(ambientOcclusionMap.rgb);
  return clamp(ao,0.0,1.0);
}

vec3 sampleNormalMap(sampler2D normalSampler, vec2 uv, mat3 spaceTransformMatrix)
{
  vec3 normalMap = texture(normalSampler,uv).xyz;
  // convert 0 to 1 range to -1 to 1
  normalMap = normalize(normalMap * 2 - 1);
  // transform normal into the space we want. as of now lighting is done in worldspace
  normalMap =  normalize(spaceTransformMatrix * normalMap);
  return normalMap;
}


#endif
