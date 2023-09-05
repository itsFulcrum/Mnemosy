#ifndef OBJECT_H
#define OBJECT_H

#include <glad/glad.h> 

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <Shader.h>
#include <Model.h>


class Object
{
public:
	glm::vec3 position	= glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 rotation	= glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 scale		= glm::vec3(1.0f, 1.0f, 1.0f);

	
	Object(std::string modelFilePath)
	{
		// std::string const& modelFilePath

		//std::string skyboxPathString = "fbx/skyboxMesh.fbx";
		char* modelPathCstr = const_cast<char*>(modelFilePath.c_str());

		m_model.Load(modelPathCstr);

		// setting default values
		position = glm::vec3(0.0f, 0.0f, 0.0f);
		rotation = glm::vec3(0.0f, 0.0f, 0.0f);
		scale = glm::vec3(1.0f, 1.0f, 1.0f);
	}

	void Render(Shader& shader, glm::mat4 viewMatrix, glm::mat4 projectionMatrix)
	{
		shader.use();




		glm::mat4 modelMatrix = M_GetTransformMatrix();

		shader.setUniformMatrix4("_modelMatrix", modelMatrix);
		shader.setUniformMatrix4("_normalMatrix", M_GetNormalMatrix(modelMatrix));
		shader.setUniformMatrix4("_viewMatrix", viewMatrix);
		shader.setUniformMatrix4("_projectionMatrix", projectionMatrix);

		m_model.Draw(shader);
	}

private:
	Model m_model;

	glm::mat4 M_GetTransformMatrix() 
	{
		glm::mat4 transformMatrix = glm::mat4(1.0f);
		transformMatrix = glm::translate(transformMatrix,position);
		transformMatrix = glm::rotate(transformMatrix, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f)); // around z axis
		transformMatrix = glm::rotate(transformMatrix, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f)); // around y axis
		transformMatrix = glm::rotate(transformMatrix, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f)); // around x axis
		transformMatrix = glm::scale(transformMatrix, scale);
		
		return transformMatrix;
	}
	
	glm::mat4 M_GetNormalMatrix(glm::mat4 transformMatrix)
	{
		return glm::transpose(glm::inverse(transformMatrix));
	}



};


#endif // !OBJECT_H
