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
		
		glm::vec3 GetPosition() { return m_position; }
		glm::vec3 GetRotationEulerAngles() { return m_rotation; }
		glm::vec3 GetScale()	{ return m_scale; }

		glm::mat4 GetTransformMatrix();
		glm::mat4 GetNormalMatrix(glm::mat4 transformMatrix);
		
		
		glm::vec3 GetForward()	{ return m_forward; }
		glm::vec3 GetRight()	{ return m_right; }
		glm::vec3 GetUp()		{ return m_up; }

		// setters
		void SetPosition(glm::vec3 position);
		void SetRotationEulerAngles(glm::vec3 rotation);
		void SetScale(glm::vec3 scale);

		void RotateAroundAxis(float angle, glm::vec3 axis);

	private:
		void RecalculateLocalVectors_Internal(); // recalc forward,right,up vectors


	private:

		glm::quat m_quatOrientation = glm::quat(0.0f,0.0f,0.0f,1.0f);
		
		glm::vec3 m_position	= glm::vec3(0.0f, 0.0f, 0.0f);
		glm::vec3 m_rotation	= glm::vec3(0.0f, 0.0f, 0.0f); // in euler angles degrees
		glm::vec3 m_scale		= glm::vec3(1.0f, 1.0f, 1.0f);

		glm::vec3 m_forward = glm::vec3(0.0f, 0.0f, -1.0f);
		glm::vec3 m_right	= glm::vec3(1.0f, 0.0f, 0.0f);
		glm::vec3 m_up		= glm::vec3(0.0f, 1.0f, 0.0f);

	};

} // mnemosy::graphics

#endif // !TRANSFORM_H
