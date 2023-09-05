#ifndef COLOR_FUNCTIONS_GLSL
#define COLOR_FUNCTIONS_GLSL


float desaturateLinear(vec3 color)
{
  float grayscale = 0.0;

  grayscale += color.r * 0.333333333;
  grayscale += color.g * 0.333333333;
  grayscale += color.b * 0.333333333;

  return grayscale;
}

float desaturateNatural(vec3 color)
{
  float grayscale = 0.0;

  grayscale += color.r * 0.333333333;
  grayscale += color.g * 0.599999999;
  grayscale += color.b * 0.111111111;

  return grayscale;
}


// could set gamma globally as well by
// enabaling glEnable(GL_FRAMEBUFFER_SRGB); for the framebuffer
// but this gives more control
// This is not a correct sRGB converstion just a simple gamma correct.

vec4 linearTosRGB(vec4 linearColor)
{
  return vec4(pow(linearColor.rgb, vec3(1/2.2f)),linearColor.a);

}

vec4 sRGBToLinear(vec4 sRGBColor)
{
    return vec4(pow(sRGBColor.rgb, vec3(2.2f) ),sRGBColor.a);
}


vec4 adjustExposure(vec4 color,float exposure)
{
  // meant for high dynamic ranges
  vec3 ex = color.rgb * pow(2,exposure);
  return vec4(ex,color.a);
}
// 53

// reinhard Tone mapping is one of the simplest tone mapping algorithms
vec4 toneMapping(vec4 hdrColor)
{
  return vec4(hdrColor.rgb / (hdrColor.rgb+vec3(1.0f) ), hdrColor.a);
}
// 57
vec4 exposureToneMapping(vec4 hdrColor, float exposure)
{
  vec3 toneMappedLDR = vec3(1.0f) - exp(-hdrColor.rgb * exposure);
  return  vec4(toneMappedLDR,hdrColor.a);
}

vec4 postProcess(vec4 color, float exposure)
{
  vec4 exposureCorrected = adjustExposure(color,exposure);
  vec4 toneMappedLDR = toneMapping(exposureCorrected);
  return linearTosRGB(toneMappedLDR);
}


#endif
