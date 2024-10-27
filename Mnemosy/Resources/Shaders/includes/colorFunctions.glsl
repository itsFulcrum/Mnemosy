#ifndef COLOR_FUNCTIONS_GLSL
#define COLOR_FUNCTIONS_GLSL

#include agx.glsl

precision highp float;

//////////// Color Correction functions
// =============================================================================================================

float grayscaleLinear(vec3 color)
{
  return (color.r * 0.333333333) + (color.g * 0.333333333) + (color.b * 0.333333333);
}
float grayscaleNatural(vec3 color)
{
  return (color.r * 0.333333333) + (color.g * 0.599999999) + (color.b * 0.111111111);
}
float grayscaleWeighted(vec3 color,float redWeight, float greenWeight, float blueWeight)
{
  return (color.r * redWeight) + (color.g * greenWeight) + (color.b * blueWeight);
}
vec3 desaturateLinear(vec3 color,float saturation)
{
    float grayscale = (color.r * 0.333333333) + (color.g * 0.333333333) + (color.b * 0.333333333);
    return mix(vec3(grayscale,grayscale,grayscale),color,saturation);
}

vec3 desaturateNatural(vec3 color, float saturation)
{
    float grayscale = (color.r * 0.333333333) + (color.g * 0.599999999) + (color.b * 0.111111111);
    return mix(vec3(grayscale,grayscale,grayscale),color,saturation);
}
vec3 desaturateWeighted(vec3 color, float redWeight, float greenWeight, float blueWeight, float saturation)
{
    float grayscale = (color.r * redWeight) + (color.g * greenWeight) + (color.b * blueWeight);
    return mix(vec3(grayscale,grayscale,grayscale),color,saturation);
}

vec4 applyExposure(vec4 color,float exposure)
{
  // meant for high dynamic ranges
  vec3 ex = color.rgb * pow(2,exposure);
  return vec4(ex,color.a);
}


//////////// Gamma Functions
// =============================================================================================================
// input is asumed to be in 0 to 1 range
float linear_to_srgb_float(float linear){

  if(linear <= 0.0031308f){
    return linear * 12.92f;
  }
    
  return 1.055f*pow(linear,(1.0f / 2.4f) ) - 0.055f;
}

// input is asumed to be in 0 to 1 range
float srgb_to_linear_float(float srgb){

  if(srgb <= 0.04045f){
    return srgb/12.92f;
  }

  return pow( (srgb + 0.055f)/ 1.055f, 2.4f);
}


// correct srgb transform functions but not super efficiant
vec4 srgb_to_linear(vec4 sRGBColor) {

    float r = srgb_to_linear_float(sRGBColor.r);
    float g = srgb_to_linear_float(sRGBColor.g);
    float b = srgb_to_linear_float(sRGBColor.b);

    return vec4(r,g,b,sRGBColor.a);
}

vec4 linear_to_srgb(vec4 linearColor)
{

  float r = linear_to_srgb_float(linearColor.r);
  float g = linear_to_srgb_float(linearColor.g);
  float b = linear_to_srgb_float(linearColor.b);

  return vec4(r,g,b,linearColor.a);
}

// cheap srgb but not 100% accurate
vec4 linear_to_srgb_cheap(vec4 linearColor)
{
  return vec4(pow(linearColor.rgb, vec3(0.454545)),linearColor.a);
}

vec4 srgb_to_linear_cheap(vec4 sRGBColor)
{
    return vec4(pow(sRGBColor.rgb, vec3(2.2f)),sRGBColor.a);
}


vec3 tonemap_agx(vec3 linearHdr){

  return agx(linearHdr);
}


// Narkowicz 2015, "ACES Filmic Tone Mapping Curve"
vec3 tonemap_aces(vec3 x) {
  const float a = 2.51;
  const float b = 0.03;
  const float c = 2.43;
  const float d = 0.59;
  const float e = 0.14;
  return clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0, 1.0);
}


// default values could be sigma=0.7, n = 1.1;
vec3 tonemap_SCurve(vec3 value, float sigma, float n)
{
  vec3 pow_value = pow(value, vec3(n));
  return pow_value / (pow_value + pow(sigma, n));
}

// reinhard Tone mapping is one of the simplest tone mapping algorithms but performs pretty good
vec3 tonemap_reinhard(vec3 hdrColor) {
  return hdrColor / (1.0 + hdrColor);
}

vec3 tonemap_filmic(vec3 x) {
  vec3 X = max(vec3(0.0), x - 0.004);
  vec3 result = (X * (6.2 * X + 0.5)) / (X * (6.2 * X + 1.7) + 0.06);
  return pow(result, vec3(2.2));
}


vec4 postProcess(vec4 color, float exposure)
{
  
  vec4 exposureCorrected = applyExposure(color,exposure);
  

  // agx
  vec3 toneMappedLDR = tonemap_agx(exposureCorrected.rgb);

  // aces
  // vec3 toneMappedLDR = tonemap_aces(exposureCorrected.rgb);


  // reihard
  // vec3 toneMappedLDR = tonemap_reinhard(exposureCorrected.rgb);
  
  // propper filmic
  // vec3 toneMappedLDR = tonemap_filmic(exposureCorrected.rgb);


  // clamped , like blender standart view transform
  //vec3 toneMappedLDR = clamp(exposureCorrected.rgb,0.0f,1.0f);


  return linear_to_srgb(vec4(toneMappedLDR.rgb,color.a));
}



//////////// Blend Modes
// =============================================================================================================
/*
vec4 blend_OpacityBlend(vec4 Base, vec4 Blend, float Opacity)
{
    return mix(Base, Out, Opacity);
}

vec4 blend_Darken(vec4 Base, vec4 Blend, float Opacity)
{
  vec4 Out = min(Blend, Base);
  return mix(Base, Out, Opacity);
}
vec4 blend_Lighten(vec4 Base, vec4 Blend, float Opacity)
{
  vec4 Out = max(Blend, Base);
  return mix(Base, Out, Opacity);
}
vec4 blend_Multiply(vec4 Base, vec4 Blend, float Opacity)
{
  vec4 Out = Base*Blend;
  return mix(Base, Out, Opacity);
}
vec4 blend_Screen(vec4 Base, vec4 Blend, float Opacity)
{
  vec4 Out = 1.0 - (1.0 - Blend) * (1.0 - Base);
  return mix(Base, Out, Opacity);
}
vec4 blend_ColorDodge(vec4 Base, vec4 Blend, float Opacity)
{
  vec4 Out = Base / (1.0 - clamp(Blend, 0.000001, 0.999999));
  return mix(Base, Out, Opacity);
}
vec4 blend_ColorBurn(vec4 Base, vec4 Blend, float Opacity) // Not testet or checked with unitys implementation
{
  vec4 Out = 1 - (1.0 - clamp(Blend, 0.000001, 0.999999)/Base;
  return mix(Base, Out, Opacity);
}
vec4 blend_Add(vec4 Base, vec4 Blend, float Opacity)
{
  vec4 Out = Base + Blend;
  return  mix(Base, Out, Opacity);
}
vec4 blend_Subtract(vec4 Base, vec4 Blend, float Opacity)
{
  vec4 Out = Base + Blend - 1;
  return  mix(Base, Out, Opacity);
}
vec4 blend_Overlay(vec4 Base, vec4 Blend, float Opacity)
{
    vec4 result1 = 1.0 - 2.0 * (1.0 - Base) * (1.0 - Blend);
    vec4 result2 = 2.0 * Base * Blend;
    vec4 zeroOrOne = step(Base, 0.5);
    vec4 Out = result2 * zeroOrOne + (1 - zeroOrOne) * result1;
    return mix(Base, Out, Opacity);
}
vec4 blend_HardLight(vec4 Base, vec4 Blend, float Opacity)
{
    vec4 result1 = 2.0 * Base * Blend;
    vec4 result2 = 1-2*(1-Base)*(1-Blend);
    vec4 zeroOrOne = step(0.5, Blend);
    vec4 Out = result2 * zeroOrOne + (1 - zeroOrOne) * result1;
    return mix(Base, Out, Opacity);
}

vec4 blend_SoftLight(vec4 Base, vec4 Blend, float Opacity)
{
    vec4 result1 = 2.0 * Base * Blend + Base * Base * (1.0 - 2.0 * Blend);
    vec4 result2 = sqrt(Base) * (2.0 * Blend - 1.0) + 2.0 * Base * (1.0 - Blend);
    vec4 zeroOrOne = step(0.5, Blend);
    vec4 Out = result2 * zeroOrOne + (1 - zeroOrOne) * result1;
    return mix(Base, Out, Opacity);
}
vec4 blend_VividLight(vec4 Base, vec4 Blend, float Opacity)
{
    vec4 result1 = 1-(1-Blend)/(2*Base);
    vec4 result2 = Blend/(2*(1-Base));
    vec4 zeroOrOne = step(0.5, Blend);
    vec4 Out = result2 * zeroOrOne + (1 - zeroOrOne) * result1;
    return mix(Base, Out, Opacity);
}
vec4 blend_HardMix(vec4 Base, vec4 Blend, float Opacity) // not sure if condition is implemented correctly should check
{
    vec4 result1 =  vec4(0,0,0,1);
    vec4 result2 = vec4(1,1,1,1);
    vec4 zeroOrOne = step(1-Base, Blend);
    vec4 Out = result2 * zeroOrOne + (1 - zeroOrOne) * result1;
    return mix(Base, Out, Opacity);
}
vec4 blend_Exclusion(vec4 Base, vec4 Blend, float Opacity)
{
  vec4 Out = Base + Blend - 2 * Base * Blend;
  return  mix(Base, Out, Opacity);
}
*/






#endif
