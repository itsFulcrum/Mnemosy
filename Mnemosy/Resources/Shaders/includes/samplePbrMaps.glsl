#ifndef SAMPLE_PBR_MAPS
#define SAMPLE_PBR_MAPS

#include colorFunctions.glsl
#include mathFunctions.glsl

precision highp float;

  vec4 sampleAlbedoAlphaMap(sampler2D albedoSampler, vec2 uv,vec4 albedoColorValue)
  {
    vec4 albedoMap = texture(albedoSampler,uv);
    //albedoMap = clamp(albedoMap,0.0,1.0);
    albedoMap = sRGBToLinear(albedoMap);

    vec3 albedoSolidColor = sRGBToLinear(albedoColorValue).rgb;

    vec3 albedoOut =  lerp(albedoMap.rgb,albedoSolidColor.rgb,albedoColorValue.w);

    return vec4(albedoOut.rgb,albedoMap.a);
  }

  vec3 sampleEmissionMap(sampler2D emissionSampler, vec2 uv,vec4 emissionColorValue, bool useEmissveAsMask)
  {
    vec4 emissionMap = texture(emissionSampler,uv);


    vec4 emissionLinear = sRGBToLinear (emissionMap);

    vec3 emissionSolidColor = sRGBToLinear(emissionColorValue).rgb;

    vec3 emissionOut = vec3(0.0f,0.0f,0.0f);

    if(useEmissveAsMask){

      emissionOut = emissionLinear.r * emissionSolidColor;
    }
    else {
      emissionOut = lerp(emissionLinear.rgb,emissionSolidColor.rgb,emissionColorValue.w);
    }


    return emissionOut;
  }

  float sampleRoughnessMap(sampler2D roughnessSampler,vec2 uv,vec2 roughnessValue)
  {
    float roughness = texture(roughnessSampler,uv).r;
    roughness = lerp(roughness,roughnessValue.x,roughnessValue.y);
    return clamp(roughness,0.0,1.0);
  }

  float sampleMetallicMap(sampler2D metallicSampler,vec2 uv,vec2 metallicValue)
  {
    float metallic = texture(metallicSampler,uv).r;
    metallic = lerp(metallic,metallicValue.x,metallicValue.y);
    return clamp(metallic,0.0,1.0);
  }


  float sampleAmbientOcclusionMap(sampler2D ambientOcclusionSampler,vec2 uv,float aoValue)
  {
    vec4 ambientOcclusionMap = texture(ambientOcclusionSampler,uv);
    //float ao = grayscaleLinear(ambientOcclusionMap.rgb);
    float ao = texture(ambientOcclusionSampler,uv).r;
    ao = lerp(ao,1.0,aoValue);
    return clamp(ao,0.0,1.0);
  }

  vec3 sampleNormalMap(sampler2D normalSampler, vec2 uv, float strength, mat3 spaceTransformMatrix,float normalValue)
  {
    vec3 normalMap = texture(normalSampler,uv).xyz;
    normalMap = lerp(normalMap.xyz, vec3(0.5f,0.5f,1.0f),normalValue);

    // convert 0 to 1 range to -1 to 1
    normalMap = normalMap * 2 - 1;//normalize(normalMap * 2 - 1);

    //normalMap = NormalStrengthSpherical(normalMap,strength); // unity version looks better than using spherical coords and is also much faster
    normalMap = vec3(normalMap.xy * strength,mix(1,normalMap.z,clamp(strength,0,1)));


    normalMap = normalize(normalMap);
    //vec3(In.xy * Strength, lerp(1, In.z, saturate(Strength)));

    // transform normal into the space we want. as of now lighting is done in worldspace
    normalMap =  normalize(spaceTransformMatrix * normalMap);
    return normalMap;
  }

  float SampleOpacityMap(sampler2D opacitySampler, vec2 uv)
  {

    float opacity = texture(opacitySampler,uv).r;

    return opacity;
  }


#endif
