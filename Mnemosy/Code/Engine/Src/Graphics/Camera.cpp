#include "Include/Graphics/Camera.h"

#include "Include/MnemosyEngine.h"

#include <glm/gtc/matrix_transform.hpp>

namespace mnemosy::graphics
{
	Camera::Camera(const unsigned int renderScreenWidth,const unsigned int renderScreenHeight) {
		m_screenWidth = renderScreenWidth;
		m_screenHeight = renderScreenHeight;
		//SetScreenSize(renderScreenWidth, renderScreenHeight);
	}


	void Camera::SetScreenSize(const unsigned int width,const unsigned int height) {
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
		projectionMatrix = glm::perspective(glm::radians(settings.fov), float(m_screenWidth) / float(m_screenHeight), settings.nearClip, settings.farClip);
		return projectionMatrix;
	}

} // !mnemosy::graphics