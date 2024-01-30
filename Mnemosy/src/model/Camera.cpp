#include "Camera.h"


Camera::Camera(unsigned int renderScreenWidth, unsigned int renderScreenHeight)
{
	m_worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
	
	position = glm::vec3(0.0f, 0.0f, 0.0f);
	m_forward = glm::vec3(0.0f, 0.0f, -1.0f);

	m_pitch = 0.0f;
	m_yaw = -90.0f;

	RecalculateLocalVectors(m_forward);
	SetScreenSize(renderScreenWidth, renderScreenHeight);
}


Camera::~Camera()
{
}

void Camera::SetScreenSize(unsigned int width, unsigned int height)
{
	m_screenWidth = width;
	m_screenHeight = height;
}


glm::mat4 Camera::GetViewMatrix()
{
	glm::mat4 viewMatrix = glm::mat4(1.0f);
	viewMatrix = glm::lookAt(position, position + m_forward, m_up);
	return viewMatrix;
}

glm::mat4 Camera::GetProjectionMatrix()
{
	glm::mat4 projectionMatrix = glm::mat4(1.0f);
	projectionMatrix = glm::perspective(glm::radians(settings.fov), float(m_screenWidth) / float(m_screenHeight), settings.nearClip, settings.farClip);
	return projectionMatrix;
}

void Camera::SetYawPitch(float yaw, float pitch)
{
	
	m_yaw = yaw;
	m_pitch = pitch;


	glm::vec3 newForward = glm::vec3(0.0f, 0.0f, 0.0f);

	newForward.x = cos(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
	newForward.y = sin(glm::radians(m_pitch));
	newForward.z = sin(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));

	RecalculateLocalVectors(newForward);

}

void Camera::RecalculateLocalVectors(glm::vec3 newForward)
{
	m_forward = glm::normalize(newForward);
	m_right = glm::normalize(glm::cross(m_forward, m_worldUp));  // normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
	m_up = glm::normalize(glm::cross(m_right, m_forward));
}
