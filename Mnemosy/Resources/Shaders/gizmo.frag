#version 450 core

// includes
#include includes/colorFunctions.glsl

// Vertex shader inputs
in vec3 color;

//outputs
out vec4 fragmentOutputColor;

void main()
{
			vec4 fragmentColorLinear = vec4(1.0f,1.0f,1.0f,1.0);
			fragmentColorLinear.rgb = color;

	////// POST PROCCESSING ========================================================================================= ////
	//// ============================================================================================================ ////
			fragmentOutputColor	= vec4(0.0,0.0,0.0,1.0);
			fragmentOutputColor = postProcess(fragmentColorLinear,0.0);
}
