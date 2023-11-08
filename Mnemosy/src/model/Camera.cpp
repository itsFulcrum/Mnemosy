#include "Camera.h"


Camera::Camera()
{
	position = glm::vec3(0.0f, 0.0f, 0.0f);
	up = glm::vec3(0.0f, 1.0f, 0.0f);
	worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
	forward = glm::vec3(0.0f, 0.0f, -1.0f);
	pitch = PITCH;
	yaw = YAW;
	moveSpeed = MOVESPEED;
	mouseSensitivity = MOUSESENSITIVITY;

	fov = FOV;
	nearClip = 0.1f;
	farClip = 512.0f;
}
Camera::~Camera()
{
}

void Camera::Init(unsigned int renderScreenWidth, unsigned int renderScreenHeight)
{
	updateScreenSize(renderScreenWidth, renderScreenHeight);
	
	updateCameraVectors();

}

void Camera::ActivateControlls(GLFWwindow* window)
{
	if (!controllsActive) 
	{
		//std::cout << "Camera::ACTIVATE_Controlls: Cursor Disabled" << std::endl;
		controllsActive = true;
		//glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	}
}
void Camera::DeactivateControlls(GLFWwindow* window)
{
	if (controllsActive)
	{
		//std::cout << "Camera::ACTIVATE_Controlls: Cursor Enabled" << std::endl;
		controllsActive = false;
		//glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	}
}

void Camera::updateScreenSize(unsigned int width, unsigned int height)
{
	m_screenWidth = width;
	m_screenHeight = height;
}

// keyboard callback
void Camera::ProcessKeyboardInput(Camera_Movement direction, float deltaTime)
{
	if (!controllsActive)
		return;

	if (cameraMode == CAMERA_MODE_FLY)
	{
		float speed = moveSpeed * deltaTime;
		if (direction == CAMERA_FORWARD)
		{
			position += speed * forward;
		}
		else if (direction == CAMERA_BACKWARD)
		{
			position -= speed * forward;
		}
		else if (direction == CAMERA_LEFT)
		{
			position -= glm::normalize(glm::cross(forward, up)) * speed;
		}
		else if (direction == CAMERA_RIGHT)
		{
			position += glm::normalize(glm::cross(forward, up)) * speed;
		}
		updateCameraVectors();
	}
}

// mouse callback
void Camera::ProessMouseInput(double xOffset, double yOffset)
{
	if (!controllsActive)
		return;


	float mouseOffsetX = (float)xOffset;
	float mouseOffsetY = (float)yOffset;
	mouseOffsetX *= mouseSensitivity;
	mouseOffsetY *= mouseSensitivity;

	if (cameraMode == CAMERA_MODE_EDIT)
	{
		position += right * mouseOffsetX;
		position += up * mouseOffsetY;

	}
	else if (cameraMode == CAMERA_MODE_FLY)
	{
		yaw += mouseOffsetX;
		pitch += mouseOffsetY;

		pitch = ClampFloat(pitch, -89.0f, 89.0f);
	}

	updateCameraVectors();
}

// scroll callback
void Camera::ProcessMouseScrollInput(float yOffset, float deltaTime)
{
	if (!controllsActive)
		return;


	if (cameraMode == CAMERA_MODE_EDIT)
	{
		position += moveSpeed * yOffset * forward;

		//if(yOffset > 0)
		//else if (yOffset < 0)
		//	position -= speed * forward;

		updateCameraVectors();
	}

	else if (cameraMode == CAMERA_MODE_FLY)
	{
	
		fov -= (float)yOffset;
		fov = ClampFloat(fov, 1.0f, 80.0f);
	
	}

}


glm::mat4 Camera::GetViewMatrix()
{
	glm::mat4 viewMatrix = glm::mat4(1.0f);
	viewMatrix = glm::lookAt(position, position + forward, up);
	return viewMatrix;
}
glm::mat4 Camera::GetProjectionMatrix()
{
	glm::mat4 projectionMatrix = glm::mat4(1.0f);
	projectionMatrix = glm::perspective(glm::radians(fov), float(m_screenWidth) / float(m_screenHeight), nearClip, farClip);
	return projectionMatrix;
}


float Camera::ClampFloat(float input, float min, float max)
{
	float output = input;
	if (output > max)
		output = max;
	else if (output < min)
		output = min;

	return output;
}
void Camera::updateCameraVectors()
{
	glm::vec3 newViewDirection = glm::vec3(0.0f, 0.0f, 0.0f);

	if (cameraMode == CAMERA_MODE_EDIT)
	{
		// direction from cam pos to focus point
		if (glm::distance(position, focusPoint) < 0.01f)
		{
			position -= 0.05f * forward;
		}
		newViewDirection = (focusPoint - position);
	}

	else if (cameraMode == CAMERA_MODE_FLY)
	{
		newViewDirection.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
		newViewDirection.y = sin(glm::radians(pitch));
		newViewDirection.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	}

	forward = glm::normalize(newViewDirection);
	right = glm::normalize(glm::cross(forward, worldUp));  // normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
	up = glm::normalize(glm::cross(right, forward));
}