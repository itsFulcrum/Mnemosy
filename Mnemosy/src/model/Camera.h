#ifndef CAMERA_H
#define CAMERA_H

#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <GLFW/glfw3.h>



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

	// Settings
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

	bool controllsActive = false;
	Camera_Mode cameraMode = CAMERA_MODE_EDIT;
	
	glm::vec3 focusPoint = glm::vec3(0.0f,0.0f,0.0f);

	// constructor with glm vectors
	Camera();
	~Camera();

	void Init(unsigned int renderScreenWidth, unsigned int renderScreenHeight);
	
	void updateScreenSize(unsigned int width, unsigned int height);
	void ActivateControlls(GLFWwindow* window);
	void DeactivateControlls(GLFWwindow* window);

	// Callbacks
	void ProcessKeyboardInput(Camera_Movement direction, float deltaTime);
	void ProessMouseInput(double xOffset, double yOffset);
	void ProcessMouseScrollInput(float yOffset, float deltaTime);


	glm::mat4 GetViewMatrix();
	glm::mat4 GetProjectionMatrix();


private:
	unsigned int m_screenWidth;
	unsigned int m_screenHeight;

	float ClampFloat(float input, float min, float max);
	void updateCameraVectors();

};




#endif // !CAMERA_H


