#ifndef TRANSFORM_H
#define TRANSFORM_H

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/glm.hpp"
#include "glm/gtc/quaternion.hpp"
#include "glm/gtx/quaternion.hpp"

namespace mnemosy::graphics
{
	struct Transform
	{
	public:
		Transform() = default;
		~Transform() = default;

		
		// Getters
		const glm::vec3 GetPosition() const { return m_position; }
		const glm::vec3 GetRotationEulerAngles() const { return m_rotationEuler; }
		const glm::vec3 GetScale()	const { return m_scale; }
		
		const glm::mat4 GetTransformMatrix() const ;
		const glm::mat4 GetNormalMatrix(const glm::mat4& transformMatrix) const;
		
		const glm::vec3 GetForward() const { return m_forward; }
		const glm::vec3 GetRight(); //	const { return m_right; }
		const glm::vec3 GetUp();

		// setters
		void SetPosition(const glm::vec3& position);
		void SetRotationEulerAngles(const glm::vec3& rotation);
		void SetScale(const glm::vec3& scale);
		void SetRotationQuaternion(const glm::quat& orientation);

		void RotateAroundAxis(const float angle, const glm::vec3& axis);

	private:
		void RecalculateForwardVector_Internal();

	private:
		glm::vec3 m_forward = glm::vec3(0.0f, 0.0f, -1.0f);

		glm::vec3 m_position	= glm::vec3(0.0f, 0.0f, 0.0f);
		glm::vec3 m_rotationEuler	= glm::vec3(0.0f, 0.0f, 0.0f); // in euler angles degrees
		glm::vec3 m_scale		= glm::vec3(1.0f, 1.0f, 1.0f);

		glm::quat m_quatOrientation = glm::quat(0.0f,0.0f,0.0f,1.0f);

	};

} // mnemosy::graphics

#endif // !TRANSFORM_H
