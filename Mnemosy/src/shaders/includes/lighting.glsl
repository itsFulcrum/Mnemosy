#ifndef PBR_LIGHTING_GLSL
#define PBR_LIGHTING_GLSL

const float LIGHTING_PI = 3.14159265359;
const float DIALECTRIC_F0 = 0.04f;
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
  vec3 lightPosition;
  vec3 lightColor;
};

vec3 Lighting_Math_Rotate_About_Axis_Radians_float(vec3 In, vec3 Axis, float Rotation)
{
    float s = sin(Rotation);
    float c = cos(Rotation);
    float one_minus_c = 1.0 - c;

    Axis = normalize(Axis);

    mat3x3 rot_mat;


    //one_minus_c * Axis.x * Axis.x + c,            one_minus_c * Axis.x * Axis.y - Axis.z * s,     one_minus_c * Axis.z * Axis.x + Axis.y * s,
    //one_minus_c * Axis.x * Axis.y + Axis.z * s,   one_minus_c * Axis.y * Axis.y + c,              one_minus_c * Axis.y * Axis.z - Axis.x * s,
    //one_minus_c * Axis.z * Axis.x - Axis.y * s,   one_minus_c * Axis.y * Axis.z + Axis.x * s,     one_minus_c * Axis.z * Axis.z + c


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

vec3 ImportanceSampleGGX(vec2 Xi, vec3 N, float roughness)
{
    float a = roughness*roughness;

    float phi = 2.0 * LIGHTING_PI * Xi.x;
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
  denom = LIGHTING_PI * denom * denom;
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

vec3 CookTorranceSpecularBRDF(vec3 position, vec3 normal,float NDOTV, vec3 viewDirection,vec3 F0,vec3 lightPosition, vec3 lightColor ,vec3 albedo,float metallic,float roughness)
{
    // the CookTorranceSpecularBRDF needs to run per lightsource to calculate the combined radiance (energy) of a given point/fragment

    vec3 lightDirection = normalize( lightPosition - position);
    vec3 halfVector = normalize(viewDirection + lightDirection);

    // light distance should only apply for non directional lights
    // for directional light we should keep light attentuation at 1;
    float lightDistance = length(lightPosition - position);
    // light attentuation using inverse squre law which is phisically correct.
    // -> may want to use constant linear quadratic equation for more controll
    //	float attenuation = 1.0 / (constantValue + linearValue * distance + quadraticValue * (distance * distance));
    float lightAttentuation = 1.0f / (lightDistance * lightDistance);
    lightAttentuation = 1.0f; // currently force to 1 to simulate directional light behavior
    vec3 radiance = lightColor * lightAttentuation;


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

    vec3 outgoingRadiance = (diffuseReflection * albedo / LIGHTING_PI + specular) * radiance * NdotL;
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
    directRadiance += CookTorranceSpecularBRDF(ld.fragmentPosition, sd.normal,NdotV, -ld.viewDirection,F0, ld.lightPosition, ld.lightColor, sd.albedo, sd.metallic, sd.roughness);


  ////// INDIRECT LIGHTING ======================================================================================== ////
  //// ============================================================================================================ ////
    vec3 F = FresnelSchlickRoughness(NdotV,F0,sd.roughness);
    vec3 kS = F;
    vec3 kD = 1.0 -kS;
    kD *= 1.0 - sd.metallic;

    vec3 R = reflect(ld.viewDirection,sd.normal);

    // INDIRECT DIFFUSE
    vec3 rotatedNormal =  Lighting_Math_Rotate_About_Axis_Radians_float(sd.normal, vec3(0.0f,1.0f,0.0f), ld.skyboxRotation);
    vec3 irradiance = texture(irradianceMap, rotatedNormal).rgb;
    vec3 diffuse = irradiance * sd.albedo;

    // INDIRECT SPECULAR
    vec3 reflectVec = reflect(ld.viewDirection, sd.normal);
    vec3 rotatedReflect = Lighting_Math_Rotate_About_Axis_Radians_float(reflectVec, vec3(0.0f,1.0f,0.0f), ld.skyboxRotation);

    vec3 prefilteredColor = textureLod(prefilterMap, rotatedReflect,  sd.roughness * MAX_REFLECTION_LOD).rgb;
    vec2 envBRDF  = texture(brdfLUT, vec2(NdotV, sd.roughness)).rg;
    vec3 specular = prefilteredColor * (F * envBRDF.x + envBRDF.y);

    vec3 indirectRadiance = (kD * diffuse + specular) * sd.ambientOcclusion;


  ////// Emissive ================================================================================================= ////
  //// ============================================================================================================ ////

    vec3 emissionRadiance = sd.emissive * sd.emissionStrength;

  ////// COMBINED LIGHTING ======================================================================================== ////
  //// ============================================================================================================ ////

    vec3 pbrShadedColor = directRadiance + indirectRadiance + emissionRadiance;
  ////// OUTPUT =================================================================================================== ////
  //// ============================================================================================================ ////

    return vec4(pbrShadedColor,sd.alpha);
}

#endif
