#version 450

#extension GL_NV_mesh_shader : require

layout(local_size_x = 1) in;
layout(triangles) out;
layout(max_vertices = 4) out;
layout(max_primitives = 2) out;

// Custom vertex output block for fragment shader
layout (location = 0) out PerVertexData
{
  vec4 color;
} v_out[];  // [max_vertices]


// Vertecies of a quad
const vec3 vertices[4] = {
  vec3(-1,-1,0), // [0] BOTTOM LEFT
  vec3(-1, 1,0), // [1] TOP LEFT
  vec3( 1,-1,0), // [2] BOTTOM RIGHT
  vec3( 1, 1,0)  // [3] TOP RIGHT
};

const vec3 colors[4] = {
  vec3(1.0,0.0,0.0), // RED
  vec3(0.0,1.0,0.0), // GREEN
  vec3(0.0,0.0,1.0), // BLUE
  vec3(1.0,1.0,1.0)  // WHITE
};


// uniform data
uniform mat4 _modelMatrix;
uniform mat4 _viewMatrix;
uniform mat4 _projectionMatrix;

uniform vec3 _VertPos; // vertex position passed as uniform..

void main()
{
  float quadSize = 0.02;
  mat4 MVP = _projectionMatrix * _viewMatrix * _modelMatrix;

  vec4 quadPos1 = MVP * vec4( _VertPos + (vertices[0] * quadSize), 1.0);
  vec4 quadPos2 = MVP * vec4( _VertPos + (vertices[1] * quadSize), 1.0);
  vec4 quadPos3 = MVP * vec4( _VertPos + (vertices[2] * quadSize), 1.0);
  vec4 quadPos4 = MVP * vec4( _VertPos + (vertices[3] * quadSize), 1.0);

  // Vertices position
  gl_MeshVerticesNV[0].gl_Position = quadPos1;
  gl_MeshVerticesNV[1].gl_Position = quadPos2;
  gl_MeshVerticesNV[2].gl_Position = quadPos3;
  gl_MeshVerticesNV[3].gl_Position = quadPos4;

  // Vertices color
  v_out[0].color = vec4(colors[0], 1.0);
  v_out[1].color = vec4(colors[1], 1.0);
  v_out[2].color = vec4(colors[2], 1.0);
  v_out[3].color = vec4(colors[3], 1.0);

  // Triangle 1
  gl_PrimitiveIndicesNV[0] = 0;
  gl_PrimitiveIndicesNV[1] = 1;
  gl_PrimitiveIndicesNV[2] = 3;
  // Triangle 2
  gl_PrimitiveIndicesNV[3] = 0;
  gl_PrimitiveIndicesNV[4] = 3;
  gl_PrimitiveIndicesNV[5] = 2;

  // Number of triangles
  gl_PrimitiveCountNV = 2;
}
