#version 450 core

// includes
#include includes/colorFunctions.glsl


// Vertex shader inputs
//in vec3 position_WS;

// inputs
uniform vec3 _lightColor;
uniform float _lightStrength;
//uniform int _lightType; // 0 = directional, 1 = point
//uniform float _lightAttentuation;
//uniform vec3 _cameraPositionWS;

//outputs
out vec4 fragmentOutputColor;


void main()
{

			vec4 fragmentColorLinear = vec4(0.0,0.0,0.0,1.0);

			// perhaps we need to do some srgb conversions here maybe
			vec3 lightColor = _lightColor * _lightStrength;

			fragmentColorLinear.rgb = lightColor;

	////// POST PROCCESSING ========================================================================================= ////
	//// ============================================================================================================ ////
			//fragmentOutputColor	= vec4(0.0,0.0,0.0,1.0);


			fragmentOutputColor = postProcess(fragmentColorLinear,0.0);

}
