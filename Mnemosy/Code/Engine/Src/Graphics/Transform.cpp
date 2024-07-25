#include "Include/Graphics/Transform.h"

#include <glm/gtc/matrix_transform.hpp>

namespace mnemosy::graphics
{
	const glm::mat4 Transform::GetTransformMatrix() const {

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

	const glm::mat4 Transform::GetNormalMatrix(const glm::mat4& transformMatrix) const {
		return glm::transpose(glm::inverse(transformMatrix));
	}

	const glm::vec3 Transform::GetRight()
	{
		return glm::normalize(glm::cross(m_forward, glm::vec3(0.0f, 1.0f, 0.0f)));
	}

	const glm::vec3 Transform::GetUp() {

		return glm::normalize(glm::cross(GetRight(), m_forward));
	}

	void Transform::SetPosition(const glm::vec3& position) { m_position = position;}

	void Transform::SetRotationEulerAngles(const glm::vec3& rotation)
	{
		m_rotationEuler = rotation;
		m_quatOrientation = glm::quat(glm::radians(m_rotationEuler)); // convert from euler angles in radians to quaternion

		RecalculateForwardVector_Internal();
	}

	void Transform::SetScale(const glm::vec3& scale) {m_scale = scale;}

	void Transform::SetRotationQuaternion(const glm::quat& orientation)
	{
		m_quatOrientation = orientation;

		RecalculateForwardVector_Internal();
	}

	void Transform::RotateAroundAxis(const float angle, const glm::vec3& axis)
	{
		glm::quat rotate = glm::angleAxis(glm::radians(angle),axis);

		m_quatOrientation =  glm::inverse(rotate) * m_quatOrientation;

		RecalculateForwardVector_Internal();
	}

	void Transform::RecalculateForwardVector_Internal()
	{
		glm::vec3 initialForward = glm::vec3(0.0, 0.0, -1.0);

		// calculate forward from quaternion orientation by rotating the inital forward by the quaternion
		glm::vec3 newForward = - glm::rotate(m_quatOrientation, initialForward); 
		m_forward = glm::normalize(newForward);
	}

} // !mnemosy::graphics