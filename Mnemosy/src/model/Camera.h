#ifndef CAMERA_H
#define CAMERA_H

#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <GLFW/glfw3.h>


struct CameraSettings
{
	float fov = 45.0f;
	float nearClip = 0.1f;
	float farClip = 500.0f;
};


class Camera
{

public:
	

	CameraSettings settings;

	glm::vec3 position = glm::vec3(0.0f, 0.0f, -1.0f);

	


	// constructor with glm vectors
	Camera(unsigned int renderScreenWidth, unsigned int renderScreenHeight);
	~Camera();

	void SetScreenSize(unsigned int width, unsigned int height);

	glm::mat4 GetViewMatrix();
	glm::mat4 GetProjectionMatrix();

	float GetYaw() { return m_yaw; }
	float GetPitch() { return m_pitch; }
	void SetYawPitch(float yaw, float pitch);

	void SetPosition(glm::vec3 Position);
	glm::vec3 GetForward() { return m_forward; }
	glm::vec3 GetRight() { return m_right; }
	glm::vec3 GetUp() { return m_up; }
	
	void RecalculateLocalVectors(glm::vec3 forward);

private:
	unsigned int m_screenWidth = 0;
	unsigned int m_screenHeight = 0;
	

	glm::vec3 m_forward = glm::vec3(0.0f, 0.0f, -1.0f);
	glm::vec3 m_right = glm::vec3(1.0f, 0.0f, 0.0f);
	glm::vec3 m_up = glm::vec3(0.0f, 1.0f, 0.0f);;

	float m_pitch = 0.0f;
	float m_yaw = -90.0f;
	glm::vec3 m_worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
};




#endif // !CAMERA_H


