#version 450 core

#define MATH_PI 3.1415926535897932384626433832795
#define MATH_INV_PI (1.0 / MATH_PI)

precision highp float;


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
		0.5f + 0.5f * atan(dir.z, dir.x) / MATH_PI,
		1.f - acos(dir.y) / MATH_PI);
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

  float sampleDelta = 0.0125; // subsample detail
  float nrSamples = 0.0;

  for(float phi = 0.0; phi < 2.0 * MATH_PI; phi += sampleDelta)
  {
      for(float theta = 0.0; theta < 0.5 * MATH_PI;theta += sampleDelta)
      {
        vec3 tangentSample = vec3(sin(theta)*cos(phi),sin(theta) * sin(phi),cos(theta));
        // tanget space to world
        vec3 sampleVec = tangentSample.x * right + tangentSample.y * up + tangentSample.z * normal;

        vec2 sampleUV = dirToUV(sampleVec);
        irradiance += texture(_equirectangularMap,sampleUV).rgb * cos(theta) * sin(theta);
        nrSamples++;
      }


  }
  irradiance = MATH_PI * irradiance * (1.0/float(nrSamples));

  //[...] // convolution code

  //FragColor = vec3(irradiance, 1.0);
  return irradiance;
}



//prefilterConvolute map

float RadicalInverse_VdC(uint bits)
{
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}
// ----------------------------------------------------------------------------
vec2 Hammersley(uint i, uint N)
{
    return vec2(float(i)/float(N), RadicalInverse_VdC(i));
}

vec3 ImportanceSampleGGX(vec2 Xi, vec3 N, float roughness)
{
    float a = roughness*roughness;

    float phi = 2.0 * MATH_PI * Xi.x;
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

float NormalDistributionGGX(float NdotH, float roughness)
{
  // disney and epic concluded that squaring roughness looks more acurate
  float a = roughness*roughness;
  float a2 = a*a;
  //float NdotH = max(dot(N,H),0.0f);
  float NdotH2 = NdotH * NdotH;

  float num = a2;
  float denom = (NdotH * (a2 - 1.0f) + 1.0f);
  denom = MATH_PI * denom * denom;
  return num / denom;
}

vec3 prefilterConvolute(int face, vec2 texCoord)
{
	vec2 texCoordNew = texCoord*2.0-1.0;
	vec3 scan = uvToXYZ(face, texCoordNew);

	vec3 N = normalize(scan);
	vec3 R = N;
	vec3 V = R;

	const uint SAMPLE_COUNT = 1024u;
	float totalWeight = 0.0;
	vec3 prefilteredColor = vec3(0.0);

	for (uint i = 0u; i < SAMPLE_COUNT; ++i)
	{
		vec2 Xi = Hammersley(i, SAMPLE_COUNT);
		vec3 H = ImportanceSampleGGX(Xi,N,_roughness);
		vec3 L = normalize(2.0 * dot(V,H) * H -V);
		float NdotL = max(dot(N,L),0.0);

		float NdotH= max(dot(N,H),0.0);
		float HdotV = max(dot(H,V),0.0);

		// used to determine a mip level to sample the invironment texture with in cases where we get artifacts.
		float D   = NormalDistributionGGX(NdotH, _roughness);
		float pdf = (D * NdotH / (4.0 * HdotV)) + 0.0001;
		float resolution = 1024.0; // resolution of source map (per face)
		float saTexel  = 4.0 * MATH_PI / (6.0 * resolution * resolution);
		float saSample = 1.0 / (float(SAMPLE_COUNT) * pdf + 0.0001);

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

void main(void)
{
    fragmentColor = vec4(1.0, 0.0, 1.0, 1.0);

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


}
