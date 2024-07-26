#ifndef COLOR_FUNCTIONS_GLSL
#define COLOR_FUNCTIONS_GLSL

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

// This is not a correct sRGB converstion just a standard simple gamma correct.

vec4 linearTosRGB(vec4 linearColor)
{
  return vec4(pow(linearColor.rgb, vec3(1/2.2f)),linearColor.a);
}

vec4 sRGBToLinear(vec4 sRGBColor)
{
    return vec4(pow(sRGBColor.rgb, vec3(2.2f)),sRGBColor.a);
}

// default values could be sigma=0.7, n = 1.1;
vec3 tonemapSCurve(vec3 value, float sigma, float n)
{
  vec3 pow_value = pow(value, vec3(n));
  return pow_value / (pow_value + pow(sigma, n));
}

// reinhard Tone mapping is one of the simplest tone mapping algorithms
vec4 toneMapping(vec4 hdrColor)
{
  return vec4(hdrColor.rgb / (hdrColor.rgb+vec3(1.0f) ), hdrColor.a);
}

vec4 exposureToneMapping(vec4 hdrColor, float exposure)
{
  vec3 toneMappedLDR = vec3(1.0f) - exp(-hdrColor.rgb * exposure);
  return  vec4(toneMappedLDR,hdrColor.a);
}

vec4 postProcess(vec4 color, float exposure)
{
  vec4 exposureCorrected = applyExposure(color,exposure);
  vec4 toneMappedLDR = toneMapping(exposureCorrected);
  return linearTosRGB(toneMappedLDR);
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
