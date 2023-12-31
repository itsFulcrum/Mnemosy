
#ifndef MESH_DATA_H
#define MESH_DATA_H


#include <glad/glad.h> 

#include <vector>


struct Vertex 
{
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec3 tangent;
	glm::vec3 bitangent;
	glm::vec3 color;
	glm::vec2 texCoords;
};

class MeshData
{
public:
	std::vector<Vertex> vertecies;
	std::vector<unsigned int> indecies;
	unsigned int vertexArrayObject;


	MeshData(std::vector<Vertex> Vertecies, std::vector<unsigned int> Indecies)
	{
		this->vertecies = Vertecies;
		this->indecies = Indecies;

		M_SetupMesh();
	}


private:
	unsigned int m_vertexBufferObject;
	unsigned int m_elementBufferObject;

	void M_SetupMesh() 
	{
		glGenVertexArrays(1, &vertexArrayObject);
		glGenBuffers(1, &m_vertexBufferObject);
		glGenBuffers(1, &m_elementBufferObject);

		glBindVertexArray(vertexArrayObject);
		glBindBuffer(GL_ARRAY_BUFFER, m_vertexBufferObject);
		
		glBufferData(GL_ARRAY_BUFFER, vertecies.size() * sizeof(Vertex), &vertecies[0], GL_STATIC_DRAW);
		
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_elementBufferObject);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indecies.size() * sizeof(unsigned int), &indecies[0], GL_STATIC_DRAW);

		
		//positions
			// attribPointer index,someBool,somebool, how much bytes,offset in array in bytes
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
		
		// normals
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex,normal));

		// Tangents
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, tangent));

		// Bi Tangents
		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, bitangent));

		// color 
		glEnableVertexAttribArray(4);
		glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, color));

		// texCoord / uvs
		glEnableVertexAttribArray(5);
		glVertexAttribPointer(5, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoords));
		
		
		glBindVertexArray(0);
	}

};

#endif // !Mesh_DATA_H