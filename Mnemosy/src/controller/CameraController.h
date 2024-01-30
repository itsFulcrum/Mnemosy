#pragma once
#include <glm/glm.hpp>
#include "Input/InputSystem.h"
#include "model/Camera.h"

enum Camera_Mode
{
	CAMERA_MODE_EDIT,
	CAMERA_MODE_FLY
};

struct CameraControllerSettings
{
	float moveSpeed = 1;
	float mouseSensitivity = 5;
};

class CameraController
{
public:
	CameraController(InputSystem& inputSystem,Camera& camera);
	~CameraController();


	void OnKeyPressed_W(double deltaSeconds);
	void OnKeyPressed_A(double deltaSeconds);
	void OnKeyPressed_S(double deltaSeconds);
	void OnKeyPressed_D(double deltaSeconds);

	void OnKeyPressed_ALT(double deltaSeconds);
	void OnKeyReleased_ALT(double deltaSeconds);


	void OnMouseScroll(double deltaSeconds, double xOffset, double yOffset);
	void OnMouseMoved(double deltaSeconds, double xPos,double yPos, double xOffset, double yOffset);

	void OnScreenSizeChanged(int width, int height);

private:
	glm::vec3 focusPoint = glm::vec3(0.0f, 0.0f, 0.0f);
private:


	void RegisterInputs();
	void UnregisterInputs();
	
	bool m_controllable = false;

	CameraControllerSettings m_settings;

	Camera_Mode m_cameraMode;
	InputSystem* m_inputSystem;
	Camera* m_camera;


	int m_onKeyPressed_W_id = -1;
	int m_onKeyPressed_A_id = -1;
	int m_onKeyPressed_S_id = -1;
	int m_onKeyPressed_D_id = -1;

	int m_onKeyPressed_ALT_id = -1;
	int m_onKeyReleased_ALT_id = -1;
	int m_onMouseScroll_id = -1;
	int m_onMouseMoved_id = -1;

	int m_onScreenSizeChanged_id = -1;
};