#ifndef MESH_DATA_H
#define MESH_DATA_H

#include <glm/glm.hpp>
#include <vector>

namespace mnemosy::graphics
{
	struct VertexData 
	{
		glm::vec3 position;
		glm::vec3 normal;
		glm::vec3 tangent;
		glm::vec3 bitangent;
		//glm::vec3 color;
		glm::vec2 texCoords;
	};

	struct MeshData
	{
		std::vector<VertexData> vertecies;
		std::vector<unsigned int> indecies;
		unsigned int vertexArrayObject = NULL;
		unsigned int vertexBufferObject = NULL;
		unsigned int elementBufferObject = NULL;
	};
} // mnemosy::graphics

#endif // !MESH_DATA_H