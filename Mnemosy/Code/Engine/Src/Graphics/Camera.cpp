#include "Include/Graphics/Camera.h"

#include "Include/MnemosyEngine.h"

#include <glm/gtc/matrix_transform.hpp>

namespace mnemosy::graphics
{
	Camera::Camera(unsigned int renderScreenWidth, unsigned int renderScreenHeight)
	{
		SetScreenSize(renderScreenWidth, renderScreenHeight);
	}

	Camera::~Camera()
	{	}

	void Camera::SetScreenSize(unsigned int width, unsigned int height)
	{
		m_screenWidth = width;
		m_screenHeight = height;
	}

	glm::mat4 Camera::GetViewMatrix()
	{
		glm::mat4 viewMatrix = glm::mat4(1.0f);
		viewMatrix = glm::lookAt(transform.GetPosition(), transform.GetPosition() + transform.GetForward(), transform.GetUp());
		return viewMatrix;
	}

	glm::mat4 Camera::GetProjectionMatrix()
	{

		glm::mat4 projectionMatrix = glm::mat4(1.0f);
		projectionMatrix = glm::perspective(glm::radians(settings.fov), float(m_screenWidth) / float(m_screenHeight), settings.nearClip, settings.farClip);
		return projectionMatrix;
	}

} // !mnemosy::graphics