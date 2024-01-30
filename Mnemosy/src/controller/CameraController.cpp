#include "CameraController.h"

#include <glm/glm.hpp>
#include <functional>


CameraController::CameraController(InputSystem& inputSystem, Camera& camera)
{
	m_inputSystem = &inputSystem;
	m_camera = &camera;
	m_cameraMode = Camera_Mode::CAMERA_MODE_FLY;

	RegisterInputs();
}

CameraController::~CameraController()
{
	UnregisterInputs();
}

void CameraController::OnMouseScroll(double deltaSeconds,double offsetX, double offsetY)
{
	if (!m_controllable) return;
	
	if (m_cameraMode == CAMERA_MODE_EDIT)
	{
		float f = m_settings.moveSpeed * offsetY;
		m_camera->position *= m_camera->GetForward() *f;

		//if(yOffset > 0)
		//else if (yOffset < 0)
		//	position -= speed * forward;

	}

	if (m_cameraMode == CAMERA_MODE_FLY)
	{
		
		float fov = m_camera->settings.fov;
		fov = fov - (float)offsetY;
		fov = glm::clamp(fov, 1.0f, 80.0f);
		m_camera->settings.fov = fov;
	}
}

void CameraController::OnKeyPressed_W(double deltaSeconds)
{
	if (!m_controllable) return;

	float speed = m_settings.moveSpeed * deltaSeconds;
	m_camera->position += speed * m_camera->GetForward();

}
void CameraController::OnKeyPressed_A(double deltaSeconds)
{
	if (!m_controllable) return;

	float speed = m_settings.moveSpeed * deltaSeconds;
	m_camera->position -= glm::normalize(glm::cross(m_camera->GetForward(), m_camera->GetUp())) * speed;

}
void CameraController::OnKeyPressed_S(double deltaSeconds)
{
	if (!m_controllable) return;

	float speed = m_settings.moveSpeed * deltaSeconds;
	m_camera->position -= speed * m_camera->GetForward();

}
void CameraController::OnKeyPressed_D(double deltaSeconds)
{
	if (!m_controllable) return;

	float speed = m_settings.moveSpeed * deltaSeconds;
	m_camera->position += glm::normalize(glm::cross(m_camera->GetForward(), m_camera->GetUp())) * speed;

}

void CameraController::OnKeyPressed_ALT(double deltaSeconds){	m_controllable = true;}
void CameraController::OnKeyReleased_ALT(double deltaSeconds){m_controllable = false;}

void CameraController::OnMouseMoved(double deltaSeconds, double xPos, double yPos, double xOffset, double yOffset)
{
	if (!m_controllable) return;


	float mouseOffsetX = (float)xOffset;
	float mouseOffsetY = (float)yOffset;
	mouseOffsetX *= m_settings.mouseSensitivity * deltaSeconds;
	mouseOffsetY *= m_settings.mouseSensitivity * deltaSeconds;

	if (m_cameraMode == CAMERA_MODE_FLY)
	{

		float yaw = m_camera->GetYaw() + mouseOffsetX;
		float pitch = m_camera->GetPitch() + mouseOffsetY;

		pitch = glm::clamp(pitch, -89.0f, 89.0f);

		m_camera->SetYawPitch(yaw,pitch);
	} 
	// weird behavior
	m_inputSystem->HandleMouseOverViewport(0,0,0,0);

}

void CameraController::OnScreenSizeChanged(int width, int height)
{
	m_camera->SetScreenSize(width,height);
}

void CameraController::RegisterInputs()
{
	m_onKeyPressed_W_id = m_inputSystem->RegisterKeyboardInput(GLFW_KEY_W, GLFW_PRESS, false, std::bind(&CameraController::OnKeyPressed_W, this, std::placeholders::_1));
	m_onKeyPressed_A_id = m_inputSystem->RegisterKeyboardInput(GLFW_KEY_A, GLFW_PRESS, false, std::bind(&CameraController::OnKeyPressed_A, this, std::placeholders::_1));
	m_onKeyPressed_S_id = m_inputSystem->RegisterKeyboardInput(GLFW_KEY_S, GLFW_PRESS, false, std::bind(&CameraController::OnKeyPressed_S, this, std::placeholders::_1));
	m_onKeyPressed_D_id = m_inputSystem->RegisterKeyboardInput(GLFW_KEY_D, GLFW_PRESS, false, std::bind(&CameraController::OnKeyPressed_D, this, std::placeholders::_1));

	m_onKeyPressed_ALT_id = m_inputSystem->RegisterKeyboardInput(GLFW_KEY_LEFT_ALT, GLFW_PRESS, true, std::bind(&CameraController::OnKeyPressed_ALT, this, std::placeholders::_1));
	m_onKeyReleased_ALT_id = m_inputSystem->RegisterKeyboardInput(GLFW_KEY_LEFT_ALT, GLFW_RELEASE, true, std::bind(&CameraController::OnKeyReleased_ALT, this, std::placeholders::_1));

	m_onMouseMoved_id = m_inputSystem->RegisterMouseCursorInput(std::bind(&CameraController::OnMouseMoved, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5));
	m_onMouseScroll_id = m_inputSystem->RegisterMouseScrollInput(std::bind(&CameraController::OnMouseScroll, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

	m_onScreenSizeChanged_id = m_inputSystem->RegisterWindowResizeInput(std::bind(&CameraController::OnScreenSizeChanged, this, std::placeholders::_1, std::placeholders::_2));
}

void CameraController::UnregisterInputs()
{
	m_inputSystem->UnregisterKeyboardInput(m_onKeyPressed_W_id);
	m_inputSystem->UnregisterKeyboardInput(m_onKeyPressed_A_id);
	m_inputSystem->UnregisterKeyboardInput(m_onKeyPressed_S_id);
	m_inputSystem->UnregisterKeyboardInput(m_onKeyPressed_D_id);
	m_inputSystem->UnregisterKeyboardInput(m_onKeyPressed_ALT_id);
	m_inputSystem->UnregisterKeyboardInput(m_onKeyReleased_ALT_id);

	m_inputSystem->UnregisterMouseCursorInput(m_onMouseMoved_id);
	m_inputSystem->UnregisterMouseScrollInput(m_onMouseScroll_id);
	
}
