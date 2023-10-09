#ifndef OBJECT_H
#define OBJECT_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <view/Shader.h>
#include <view/ModelData.h>


class Object
{
public:
	glm::vec3 position	= glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 rotation	= glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 scale		= glm::vec3(1.0f, 1.0f, 1.0f);

	ModelData modelData;


public:
	Object(){}
	~Object(){}

	glm::mat4 GetTransformMatrix() 
	{
		// not super efficiant as we generate it every frame, 
		// coud easily cache it and only update once it changes
		glm::mat4 transformMatrix = glm::mat4(1.0f);
		transformMatrix = glm::translate(transformMatrix,position);
		transformMatrix = glm::rotate(transformMatrix, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f)); // around z axis
		transformMatrix = glm::rotate(transformMatrix, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f)); // around y axis
		transformMatrix = glm::rotate(transformMatrix, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f)); // around x axis
		transformMatrix = glm::scale(transformMatrix, scale);
		
		return transformMatrix;
	}
	
	glm::mat4 GetNormalMatrix(glm::mat4 transformMatrix)
	{
		return glm::transpose(glm::inverse(transformMatrix));
	}

	glm::vec3 GetForward()
	{
		glm::vec3 forward = glm::vec3(0.0f, 0.0f, 0.0f);
		float Yaw =  glm::radians(rotation.y);
		float Pitch = glm::radians(rotation.z);

		forward.x = cos(Pitch) * sin(Yaw);
		forward.y = -sin(Pitch);
		forward.z = cos(Pitch) * cos(Yaw);

		forward = glm::normalize(forward);
		return forward;
	}

private:


};


#endif // !OBJECT_H
