#ifndef PBR_LIGHTING_GLSL
#define PBR_LIGHTING_GLSL

// depends on to compile
#include mathFunctions.glsl
#include pbrLightingTerms.glsl
#include colorFunctions.glsl

//const float LIGHTING_PI = 3.14159265359;
//const float DIALECTRIC_F0 = 0.04f;
const float MAX_REFLECTION_LOD = 4.0;

struct SurfaceData
{
  vec3 albedo;
  vec3 normal;
  vec3 emissive;
  float emissionStrength;
  float roughness;
  float metallic;
  float ambientOcclusion;
  float alpha;
};

struct LightingData
{
  vec3 fragmentPosition;
  vec3 viewDirection;
  float skyboxRotation;
  float skyboxExposure;
  vec3 lightPosition;
  vec3 lightColor;
  int lightType;
  float lightAttentuation;
};


vec3 CookTorranceSpecularBRDF(vec3 position, vec3 normal,float NDOTV, vec3 viewDirection,vec3 F0, int lightType, float lightAttentuation, vec3 lightPosition, vec3 lightColor ,vec3 albedo,float metallic,float roughness)
{
    // the CookTorranceSpecularBRDF needs to run per lightsource to calculate the combined radiance (energy) of a given point/fragment

    // LIGHT SETUP =======================================================================
    // for directional light, lights forward vector is passed into the lightPositon uniform
    vec3 lightDirection = normalize(lightPosition);


    float attentuation = 1.0f; // fallof // for  directional light = 1, no fallof;

    if(lightType == 1) // if Point Light
    {
      lightDirection = normalize( lightPosition - position);

      // light distance should only apply for non directional lights
      float lightDistance = length(lightPosition - position);

      //attentuation = 1.0f / (lightDistance * lightDistance); // inverse square law would be phisically accurate so attention of 1;
      attentuation = 1.0f / (lightAttentuation * (lightDistance * lightDistance));
    }


    vec3 radiance = lightColor * attentuation;


    vec3 halfVector = normalize(viewDirection + lightDirection);
    float NdotV = NDOTV;
    float NdotL = max(dot(normal,lightDirection),0.0f);
    float NdotH = max(dot(normal,halfVector),0.0f);
    float HdotV = max(dot(halfVector,viewDirection) , 0.0f);

    float cosTheta = HdotV;
    vec3 fresnel = FresnelSchlick(cosTheta ,F0);

    float normalDistribution = NormalDistributionGGX(NdotH,roughness);
    float geometry = GeometrySmith(NdotV,NdotL,roughness);

    // cook torrance brdf equation
    vec3 numerator = normalDistribution * geometry * fresnel;
    float denominator = 4.0f * NdotV * NdotL + 0.0001f;
    vec3 specular = numerator / denominator;

    // this part makes sure that light energy that gets refracted does not get reflected
    // thus keeping energy conservation.
    // as metallic surfaces only reflect and not refract light we enforce a value of 0
    // for refraction when metallic is 0
    vec3 specularReflection = fresnel; // commonly denoted as kS
    vec3 diffuseReflection = vec3(1.0f) - specularReflection; // commonly denoted as kD
    diffuseReflection = diffuseReflection * (1.0f - metallic);

    vec3 outgoingRadiance = (diffuseReflection * albedo / MATH_PI + specular) * radiance * NdotL;
    return outgoingRadiance;
}



vec4 lightingPBR(SurfaceData sd, LightingData ld,samplerCube irradianceMap,samplerCube prefilterMap, sampler2D brdfLUT)
{



  ////// PRECOMPUDED TERMS ======================================================================================== ////
  //// ============================================================================================================ ////
    // precompude some terms here already for performace as they are used frequently throughout

      vec3 F0 = mix(vec3(DIALECTRIC_F0),sd.albedo,sd.metallic);
      float NdotV = max(dot(sd.normal, -ld.viewDirection), 0.0);

  //// DIRECT LIGHTING ============================================================================================ ////
  //// ============================================================================================================ ////

    vec3 directRadiance = vec3(0.0f); // commonly denoted as Lo
    // if more then one lightsource then
    // loop over all non directional lightsources
    // currently only 1 supported
    directRadiance += CookTorranceSpecularBRDF(ld.fragmentPosition, sd.normal,NdotV, -ld.viewDirection,F0, ld.lightType,ld.lightAttentuation, ld.lightPosition, ld.lightColor, sd.albedo, sd.metallic, sd.roughness);


  ////// INDIRECT LIGHTING ======================================================================================== ////
  //// ============================================================================================================ ////
    vec3 F = FresnelSchlickRoughness(NdotV,F0,sd.roughness);
    vec3 kS = F;
    vec3 kD = 1.0 -kS;
    kD *= 1.0 - sd.metallic;

    vec3 R = reflect(ld.viewDirection,sd.normal);


    // INDIRECT DIFFUSE
    vec3 rotatedNormal =  Rotate_About_Axis_Radians_float(sd.normal, vec3(0.0f,1.0f,0.0f), ld.skyboxRotation);
    vec3 irradiance = textureLod(irradianceMap, rotatedNormal,1).rgb;
    irradiance = applyExposure(vec4(irradiance,1.0f), ld.skyboxExposure).rgb;
    vec3 diffuse = irradiance * sd.albedo;

    // INDIRECT SPECULAR
    vec3 reflectVec = reflect(ld.viewDirection, sd.normal);
    vec3 rotatedReflect = Rotate_About_Axis_Radians_float(reflectVec, vec3(0.0f,1.0f,0.0f), ld.skyboxRotation);

    vec3 prefilteredColor = textureLod(prefilterMap, rotatedReflect,  sd.roughness * MAX_REFLECTION_LOD).rgb;
    prefilteredColor = applyExposure(vec4(prefilteredColor,1.0f), ld.skyboxExposure).rgb;
    vec2 envBRDF  = texture(brdfLUT, vec2(NdotV, sd.roughness)).rg;
    vec3 specular = prefilteredColor * (F * envBRDF.x + envBRDF.y);

    vec3 indirectRadiance = (kD * diffuse + specular) * sd.ambientOcclusion;


  ////// Emissive ================================================================================================= ////
  //// ============================================================================================================ ////

    vec3 emissionRadiance = sd.emissive * sd.emissionStrength;

  ////// COMBINED LIGHTING ======================================================================================== ////
  //// ============================================================================================================ ////

    vec3 pbrShadedColor = directRadiance + indirectRadiance + emissionRadiance;
    //pbrShadedColor = irradiance;

  ////// OUTPUT =================================================================================================== ////
  //// ============================================================================================================ ////

    return vec4(pbrShadedColor,sd.alpha);
}

#endif
