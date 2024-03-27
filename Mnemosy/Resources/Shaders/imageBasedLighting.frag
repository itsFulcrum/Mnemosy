#version 450 core

precision highp float;

#include includes/mathFunctions.glsl
#include includes/pbrLightingTerms.glsl


const float irradiance_sampleDelta = 0.0125; // subsample detail - default 0.0125;

const uint prefilter_SAMPLE_COUNT = 1000u;
const float prefilter_RESOLUTION = 1000.0; // improves quality

in vec2 texCoord;
out vec4 fragmentColor;

uniform sampler2D _equirectangularMap;
uniform int _mode = 0;
uniform int _currentFace;
uniform float _roughness;

vec3 uvToXYZ(int face, vec2 uv)
{
	if(face == 0)
		return vec3(     1.f,   uv.y,    -uv.x);

	else if(face == 1)
		return vec3(    -1.f,   uv.y,     uv.x);

	else if(face == 2)
		return vec3(   +uv.x,   -1.f,    +uv.y);

	else if(face == 3)
		return vec3(   +uv.x,    1.f,    -uv.y);

	else if(face == 4)
		return vec3(   +uv.x,   uv.y,      1.f);

	else //if(face == 5)
	{	return vec3(    -uv.x,  +uv.y,     -1.f);}
}

vec2 dirToUV(vec3 dir)
{
	return vec2(
		0.5f + 0.5f * atan(dir.z, dir.x) / PI,
		1.f - acos(dir.y) / PI);
}


// For the brdf lut we calculate k slightly differently than during cookToorance shading
float brdfLutGeometrySmith(float NdotV, float NdotL, float roughness)
{
  	float a = roughness*roughness;
		float k = (a * a) / 2.0;

    float ggx2 = GeometrySchlickGGX(NdotV, k);
    float ggx1 = GeometrySchlickGGX(NdotL, k);

    return ggx1 * ggx2;
}

vec3 equirectangularToCubeMap(int face, vec2 texCoord)
{
	vec2 texCoordNew = texCoord*2.0-1.0;
	vec3 scan = uvToXYZ(face, texCoordNew);
	vec3 direction = normalize(scan);
	vec2 src = dirToUV(direction);

	return texture(_equirectangularMap, src).rgb;
}

vec3 irradianceConvolute(int face, vec2 texCoord)
{

  vec2 texCoordNew = texCoord*2.0-1.0;
	vec3 scan = uvToXYZ(face, texCoordNew); // prob just position
	vec3 direction = normalize(scan);
	//vec2 src = dirToUV(direction);


  // the sample direction equals the hemisphere's orientation
  vec3 normal =  direction;//normalize(scan);

  vec3 irradiance = vec3(0.0);

  vec3 up    = vec3(0.0,1.0,0.0);
  vec3 right = normalize(cross(up, normal));
  up         = normalize(cross(normal, right));

  //float sampleDelta = 0.0125; // subsample detail
  float nrSamples = 0.0;

  for(float phi = 0.0; phi < 2.0 * PI; phi += irradiance_sampleDelta)
  {
      for(float theta = 0.0; theta < 0.5 * PI; theta += irradiance_sampleDelta)
      {
        vec3 tangentSample = vec3(sin(theta)*cos(phi),sin(theta) * sin(phi),cos(theta));
        // tanget space to world
        vec3 sampleVec = tangentSample.x * right + tangentSample.y * up + tangentSample.z * normal;

        vec2 sampleUV = dirToUV(sampleVec);
        irradiance += texture(_equirectangularMap,sampleUV).rgb * cos(theta) * sin(theta);
        nrSamples++;
      }


  }
  irradiance = PI * irradiance * (1.0/float(nrSamples));

  //[...] // convolution code

  //FragColor = vec3(irradiance, 1.0);
  return irradiance;
}

vec3 prefilterConvolute(int face, vec2 texCoord)
{
	vec2 texCoordNew = texCoord*2.0-1.0;
	vec3 scan = uvToXYZ(face, texCoordNew);

	vec3 N = normalize(scan);
	vec3 R = N;
	vec3 V = R;

	//const uint SAMPLE_COUNT = 2000u;
	//float resolution = 4000.0; // resolution of source map (per face)

	float totalWeight = 0.0;
	vec3 prefilteredColor = vec3(0.0);

	for (uint i = 0u; i < prefilter_SAMPLE_COUNT; ++i)
	{
		vec2 Xi = Hammersley(i, prefilter_SAMPLE_COUNT);
		vec3 H = ImportanceSampleGGX(Xi,N,_roughness);
		vec3 L = normalize(2.0 * dot(V,H) * H -V);
		float NdotL = max(dot(N,L),0.0);

		float NdotH= max(dot(N,H),0.0);
		float HdotV = max(dot(H,V),0.0);

		// used to determine a mip level to sample the invironment texture with in cases where we get artifacts.
		float D   = NormalDistributionGGX(NdotH, _roughness);
		float pdf = (D * NdotH / (4.0 * HdotV)) + 0.0001;
		float saTexel  = 4.0 * PI / (6.0 * prefilter_RESOLUTION * prefilter_RESOLUTION);
		float saSample = 1.0 / (float(prefilter_SAMPLE_COUNT) * pdf + 0.0001);

		float mipLevel = _roughness == 0.0 ? 0.0 : 0.5 * log2(saSample / saTexel);


		if(NdotL > 0.0)
		{
			vec2 uvSample = dirToUV(L);
			prefilteredColor += textureLod(_equirectangularMap, uvSample,mipLevel).rgb * NdotL;
			totalWeight += NdotL;
		}
	}
	prefilteredColor = prefilteredColor / totalWeight;

	return prefilteredColor;
}

vec2 IntegrateBRDF(float NdotV, float roughness)
{
    vec3 V;
    V.x = sqrt(1.0 - NdotV*NdotV);
    V.y = 0.0;
    V.z = NdotV;

    float A = 0.0;
    float B = 0.0;

    vec3 N = vec3(0.0, 0.0, 1.0);

    const uint SAMPLE_COUNT = 1024u;
    for(uint i = 0u; i < SAMPLE_COUNT; ++i)
    {
        vec2 Xi = Hammersley(i, SAMPLE_COUNT);
        vec3 H  = ImportanceSampleGGX(Xi, N, roughness);
        vec3 L  = normalize(2.0 * dot(V, H) * H - V);

        float NdotL = max(L.z, 0.0);
        float NdotH = max(H.z, 0.0);
        float VdotH = max(dot(V, H), 0.0);

        if(NdotL > 0.0)
        {
            float G = brdfLutGeometrySmith(NdotV,NdotL, roughness);
            float G_Vis = (G * VdotH) / (NdotH * NdotV);
            float Fc = pow(1.0 - VdotH, 5.0);

            A += (1.0 - Fc) * G_Vis;
            B += Fc * G_Vis;
        }
    }
    A /= float(SAMPLE_COUNT);
    B /= float(SAMPLE_COUNT);
    return vec2(A, B);
}

// summary
// This shader is used to render images used for image based lighting as described in the paper by unreal and used by unreal engine 4
/*
depending on the mode the shader will:
_mode = 0: Render an equirectangular image to a cubemap format.
_mode = 1: Render an equirectangular image to an irradiance convoluted cubemap used for adding indirect lighting in pbr shader
_mode = 2: Render an equirectangular image to a prefilter convoluted cubemap used for sampling reflections in pbr shader.
_mode = 3: Render a brdf lut texture used in pbr shader to lookup precalculated fresnel. // at least i think thats what its doing .. kinda forgot tbh
*/

void main(void)
{
    fragmentColor = vec4(0.0, 0.0, 0.0, 0.0);

		if(_mode == 0)
		{
			fragmentColor.rgb = equirectangularToCubeMap(_currentFace, texCoord);
		}
    else if (_mode == 1)
    {
      fragmentColor.rgb = irradianceConvolute(_currentFace, texCoord);
    }
		else if(_mode == 2)
		{
			fragmentColor.rgb = prefilterConvolute(_currentFace,texCoord);
		}
		else if(_mode == 3)
		{
			fragmentColor.rg = IntegrateBRDF(texCoord.x, texCoord.y);
		}


}
