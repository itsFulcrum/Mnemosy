#ifndef SAMPLE_PBR_MAPS
#define SAMPLE_PBR_MAPS

// ==== > REQURES colorFunctions.glsl to be included first

vec4 sampleAlbedoAlphaMap(sampler2D albedoSampler, vec2 uv,vec4 albedoColorValue)
{
  vec4 albedoMap = texture(albedoSampler,uv);
  albedoMap = clamp(albedoMap,0.0,1.0);
  albedoMap = sRGBToLinear (albedoMap);

  vec3 albedoSolidColor = sRGBToLinear (albedoColorValue).rgb;

  vec3 albedoOut =  lerp(albedoMap.rgb,albedoSolidColor.rgb,albedoColorValue.w);

  return vec4(albedoOut.rgb,albedoMap.a);
}
vec3 sampleEmissionMap(sampler2D emissionSampler, vec2 uv,vec4 emissionColorValue)
{
  vec4 emissionMap = texture(emissionSampler,uv);
  emissionMap = clamp(emissionMap,0.0,1.0);
  emissionMap = sRGBToLinear (emissionMap);
  //vec3 emissionSolidColor = vec3(0.0,0.0,0.0);
  vec3 emissionSolidColor = sRGBToLinear(emissionColorValue).rgb;
  vec3 emissionOut = lerp(emissionMap.rgb,emissionSolidColor.rgb,emissionColorValue.w);
  return emissionOut;
}

float sampleRoughnessMap(sampler2D roughnessSampler,vec2 uv,vec2 roughnessValue)
{
  vec4 roughnessMap = texture(roughnessSampler,uv);
  float roughness = desaturateLinear(roughnessMap.rgb);
  roughness = lerp(roughness,roughnessValue.x,roughnessValue.y);
  return clamp(roughness,0.0,1.0);
}
float sampleMetallicMap(sampler2D metallicSampler,vec2 uv,vec2 metallicValue)
{
  vec4 metallicMap = texture(metallicSampler,uv);
  float metallic = desaturateLinear(metallicMap.rgb);
  metallic = lerp(metallic,metallicValue.x,metallicValue.y);
  return clamp(metallic,0.0,1.0);
}

float sampleAmbientOcclusionMap(sampler2D ambientOcclusionSampler,vec2 uv,float aoValue)
{
  vec4 ambientOcclusionMap = texture(ambientOcclusionSampler,uv);
  float ao = desaturateLinear(ambientOcclusionMap.rgb);
  ao = lerp(ao,1.0,aoValue);
  return clamp(ao,0.0,1.0);
}

vec3 sampleNormalMap(sampler2D normalSampler, vec2 uv, mat3 spaceTransformMatrix,float normalValue)
{
  vec3 normalMap = texture(normalSampler,uv).xyz;
  normalMap = lerp(normalMap.xyz, vec3(0.5f,0.5f,1.0f),normalValue);
  // convert 0 to 1 range to -1 to 1
  normalMap = normalize(normalMap * 2 - 1);
  // transform normal into the space we want. as of now lighting is done in worldspace
  normalMap =  normalize(spaceTransformMatrix * normalMap);
  return normalMap;
}


#endif
