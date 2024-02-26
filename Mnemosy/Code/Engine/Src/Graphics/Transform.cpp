#include "Include/Graphics/Transform.h"

#include <glm/gtc/matrix_transform.hpp>

namespace mnemosy::graphics
{
	glm::mat4 Transform::GetTransformMatrix()
	{
		glm::mat4 translationMat = glm::translate(glm::mat4(1.0f), m_position);
		glm::mat4 rotationMat = glm::toMat4(m_quatOrientation); // convert quaternion to rotation matrix
		glm::mat4 scaleMat = glm::scale(glm::mat4(1.0f), m_scale);

		glm::mat4 transformMatrix = translationMat * rotationMat * scaleMat;

		// when using eulera angles
		//transformMatrix = glm::rotate(transformMatrix, glm::radians(m_rotation.z), glm::vec3(0.0f, 0.0f, 1.0f)); // around z axis
		//transformMatrix = glm::rotate(transformMatrix, glm::radians(m_rotation.y), glm::vec3(0.0f, 1.0f, 0.0f)); // around y axis
		//transformMatrix = glm::rotate(transformMatrix, glm::radians(m_rotation.x), glm::vec3(1.0f, 0.0f, 0.0f)); // around x axis
		//transformMatrix = glm::scale(transformMatrix, m_scale);

		return transformMatrix;
	}

	glm::mat4 Transform::GetNormalMatrix(glm::mat4 transformMatrix)
	{
		return glm::transpose(glm::inverse(transformMatrix));
	}

	void Transform::SetPosition(glm::vec3 position) { m_position = position;}

	void Transform::SetRotationEulerAngles(glm::vec3 rotation)
	{
		m_rotation = rotation;
		m_quatOrientation = glm::quat(glm::radians(m_rotation)); // convert from euler angles in radians to quaternion

		RecalculateLocalVectors_Internal();
	}

	void Transform::SetScale(glm::vec3 scale) {m_scale = scale;}

	void Transform::RotateAroundAxis(float angle, glm::vec3 axis)
	{
		glm::quat rotate = glm::angleAxis(glm::radians(angle),axis);

		m_quatOrientation =  glm::inverse(rotate) * m_quatOrientation;

		RecalculateLocalVectors_Internal();
	}

	void Transform::RecalculateLocalVectors_Internal()
	{
		glm::vec3 initialForward = glm::vec3(0.0, 0.0, -1.0);
		glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f);


		glm::vec3 newForward = glm::vec3(0.0f, 0.0f, 0.0f);
		newForward = - glm::rotate(m_quatOrientation, initialForward); // calculate forward from quaternion orientation by rotating the inital forward by the quaternion

		m_forward = glm::normalize(newForward);
		m_right = glm::normalize(glm::cross(m_forward, worldUp));
		m_up = glm::normalize(glm::cross(m_right,m_forward));
	}

} // !mnemosy::graphics