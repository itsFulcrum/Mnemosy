#ifndef SCENE_OBJECT_H
#define SCENE_OBJECT_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>


class SceneObject
{
public:


	void SetPosition(float x, float y, float z) {
		m_position = glm::vec3(x, y, z);
		M_UpdateTransformMatrix();
	}
	void SetPositionX(float x) {
		m_position.x = x;
		M_UpdateTransformMatrix();
	}
	void SetPositionY(float y) {
		m_position.y = y;
		M_UpdateTransformMatrix();
	}
	void SetPositionZ(float z) {
		m_position.z = z;
		M_UpdateTransformMatrix();
	}


	void SetRotationEuler(float x, float y, float z)
	{
		m_rotation = glm::vec3(x, y, z);
		M_UpdateTransformMatrix();
	}
	void SetRotationEulerX(float x)
	{
		m_rotation.x = x;
		M_UpdateTransformMatrix();
	}
	void SetRotationEulerY(float y)
	{
		m_rotation.y = y;
		M_UpdateTransformMatrix();
	}
	void SetRotationEulerZ(float z)
	{
		m_rotation.z = z;
		M_UpdateTransformMatrix();
	}


	void SetScale(float x, float y, float z)
	{
		m_scale = glm::vec3(x, y, z);
		M_UpdateTransformMatrix();
	}
	void SetScaleX(float x)
	{
		m_scale.x = x;
		M_UpdateTransformMatrix();
	}
	void SetScaleY(float y)
	{
		m_scale.y = y;
		M_UpdateTransformMatrix();
	}
	void SetScaleZ(float z)
	{
		m_scale.z = z;
		M_UpdateTransformMatrix();
	}


	glm::mat4 GetTransformMatrix()
	{
		return m_transformMatrix;
	}

	glm::mat4 GetNormalMatrix(glm::mat4 transformMatrix)
	{
		return glm::transpose(glm::inverse(transformMatrix));
	}
private:
	void M_UpdateTransformMatrix()
	{
		glm::mat4 transformMatrix = glm::mat4(1.0f);
		transformMatrix = glm::translate(transformMatrix, m_position);
		transformMatrix = glm::rotate(transformMatrix, glm::radians(m_rotation.z), glm::vec3(0.0f, 0.0f, 1.0f)); // around z axis
		transformMatrix = glm::rotate(transformMatrix, glm::radians(m_rotation.y), glm::vec3(0.0f, 1.0f, 0.0f)); // around y axis
		transformMatrix = glm::rotate(transformMatrix, glm::radians(m_rotation.x), glm::vec3(1.0f, 0.0f, 0.0f)); // around x axis
		transformMatrix = glm::scale(transformMatrix, m_scale);

		m_transformMatrix = transformMatrix;
	}

private:
	glm::mat4 m_transformMatrix = glm::mat4(1.0);


	glm::vec3 m_position = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 m_rotation = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 m_scale = glm::vec3(1.0f, 1.0f, 1.0f);

};
#endif // !SCENE_OBJECT_H
