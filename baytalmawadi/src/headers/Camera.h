#ifndef CAMERA_H
#define CAMERA_H


//#include<glad/glad.h> // why need?
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>





//glm::vec3 cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f); // looking at world origin
//glm::vec3 cameraDirection = glm::normalize(cameraPos - cameraTarget);
//glm::vec3 cameraRight = glm::normalize(glm::cross(up, cameraDirection));
//glm::vec3 cameraUp = glm::cross(cameraRight, cameraDirection);



// using this to abstrct it from any window system
enum Camera_Movement {
	CAMERA_FORWARD,
	CAMERA_BACKWARD,
	CAMERA_LEFT,
	CAMERA_RIGHT
};


enum Camera_Mode
{
	CAMERA_MODE_EDIT,
	CAMERA_MODE_FLY
};
// Default camera values
const float YAW = -90.0f;
const float PITCH = 0.0f;
const float MOVESPEED = 2.5f;
const float MOUSESENSITIVITY = 0.1f;
const float FOV = 45.0f;

class Camera
{
public:
	float pitch = 0.0f;
	float yaw = -90.0f;

	// options
	float fov;
	float nearClip;
	float farClip;

	float moveSpeed;
	float mouseSensitivity;

	glm::vec3 position; 
	glm::vec3 up;
	glm::vec3 forward;
	glm::vec3 right;
	glm::vec3 worldUp;

	Camera_Mode cameraMode = CAMERA_MODE_EDIT;
	
	glm::vec3 focusPoint = glm::vec3(0.0f,0.0f,0.0f);

	// constructor with glm vectors
	Camera(unsigned int renderScreenWidth, unsigned int renderScreenHeight)
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

		updateScreenSize(renderScreenWidth, renderScreenHeight);
		updateCameraVectors();
	}
	
	glm::mat4 GetViewMatrix() 
	{
		glm::mat4 viewMatrix = glm::mat4(1.0f);
		viewMatrix = glm::lookAt(position, position + forward, up);
		return viewMatrix;

	}

	glm::mat4 GetProjectionMatrix()
	{
		glm::mat4 projectionMatrix = glm::mat4(1.0f);
		projectionMatrix = glm::perspective(glm::radians(fov), float(m_screenWidth) / float(m_screenHeight), nearClip, farClip);
		return projectionMatrix;
	}

	void updateScreenSize(unsigned int width, unsigned int height)
	{
		m_screenWidth = width;
		m_screenHeight = height;
	}
	// keyboard callback
	void ProcessKeyboardInput(Camera_Movement direction,float deltaTime)
	{
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
	void ProessMouseInput(double xOffset, double yOffset) 
	{

		float mouseOffsetX = (float)xOffset;
		float mouseOffsetY = (float)yOffset;
		mouseOffsetX *= mouseSensitivity;
		mouseOffsetY *= mouseSensitivity;

		if (cameraMode == CAMERA_MODE_EDIT) 
		{
			position += right * mouseOffsetX ;
			position += up * mouseOffsetY ;

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
	void ProcessMouseScrollInput(float yOffset,float deltaTime)
	{

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

private:
	unsigned int m_screenWidth;
	unsigned int m_screenHeight;

	float ClampFloat(float input, float min, float max)
	{
		float output = input;
		if (output > max)
			output = max;
		else if (output < min)
			output = min;
		
		return output;
	}
	void updateCameraVectors()
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

};


#endif // !CAMERA_H


