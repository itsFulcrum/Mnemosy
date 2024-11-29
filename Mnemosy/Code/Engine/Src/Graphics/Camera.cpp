#include "Include/Graphics/Camera.h"

#include "Include/MnemosyEngine.h"

#include <glm/gtc/matrix_transform.hpp>

namespace mnemosy::graphics
{
	Camera::Camera(const uint16_t renderScreenWidth,const uint16_t renderScreenHeight) {
		m_screenWidth = renderScreenWidth;
		m_screenHeight = renderScreenHeight;
	}


	void Camera::SetScreenSize(const uint16_t width,const uint16_t height) {
		m_screenWidth = width;
		m_screenHeight = height;
	}

	const glm::mat4 Camera::GetViewMatrix() {
		glm::mat4 viewMatrix = glm::mat4(1.0f);
		viewMatrix = glm::lookAt(transform.GetPosition(), transform.GetPosition() + transform.GetForward(), transform.GetUp());
		return viewMatrix;
	}

	const glm::mat4 Camera::GetProjectionMatrix() {

		glm::mat4 projectionMatrix = glm::mat4(1.0f);
		//												field of view (45)							nearClip (0.1f)	FarClip (500)
		projectionMatrix = glm::perspective(glm::radians(45.0f), float(m_screenWidth) / float(m_screenHeight), 0.1f, 500.0f);
		return projectionMatrix;
	}

} // !mnemosy::graphics