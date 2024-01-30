#include "SceneInputController.h"
#include <iostream>


SceneInputController::SceneInputController(InputSystem& inputSystem,float& envrionmentRotation)
{
	m_inputSystem = &inputSystem;
	m_envrionmentRotation = &envrionmentRotation;
	RegisterInputs();

}

SceneInputController::~SceneInputController()
{
	UnregisterInputs();
}



void SceneInputController::OnKeyPressed_LEFT_SHIFT(double deltaSeconds)
{
	m_SHIFT_pressed = true;
}

void SceneInputController::OnKeyReleased_LEFT_SHIFT(double deltaSeconds)
{
	m_SHIFT_pressed = false;
}

void SceneInputController::OnMouseButtonPressed_RIGHT(double deltaSeconds)
{
	m_RMB_pressed = true;
}

void SceneInputController::OnMouseButtonReleased_RIGHT(double deltaSeconds)
{
	m_RMB_pressed = false;
}

void SceneInputController::OnMouseMoved(double deltaSeconds, double xPos, double yPos, double xOffset, double yOffset)
{

	if (!m_SHIFT_pressed) return;

	if (!m_RMB_pressed) return;

	
	m_inputSystem->HandleMouseOverViewport(0, 0, 0, 0);

	float offset = xOffset * m_envrionmentRotationSpeed * deltaSeconds;
	*m_envrionmentRotation += offset;

}


void SceneInputController::RegisterInputs()
{
	m_onKeyPressed_SHIFT_id = m_inputSystem->RegisterKeyboardInput(GLFW_KEY_LEFT_SHIFT, GLFW_PRESS, true, std::bind(&SceneInputController::OnKeyPressed_LEFT_SHIFT, this, std::placeholders::_1));
	m_onKeyReleased_SHIFT_id = m_inputSystem->RegisterKeyboardInput(GLFW_KEY_LEFT_SHIFT, GLFW_RELEASE, true, std::bind(&SceneInputController::OnKeyReleased_LEFT_SHIFT, this, std::placeholders::_1));

	m_onMouseButtonPressed_RIGHT_id = m_inputSystem->RegisterMouseButtonInput(GLFW_MOUSE_BUTTON_RIGHT, GLFW_PRESS, std::bind(&SceneInputController::OnMouseButtonPressed_RIGHT, this, std::placeholders::_1));
	m_onMouseButtonPressed_RIGHT_id = m_inputSystem->RegisterMouseButtonInput(GLFW_MOUSE_BUTTON_RIGHT, GLFW_RELEASE, std::bind(&SceneInputController::OnMouseButtonReleased_RIGHT, this, std::placeholders::_1));

	m_onMouseMoved_id = m_inputSystem->RegisterMouseCursorInput(std::bind(&SceneInputController::OnMouseMoved, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5));
}

void SceneInputController::UnregisterInputs()
{
	m_inputSystem->UnregisterKeyboardInput(m_onKeyPressed_SHIFT_id);
	m_inputSystem->UnregisterKeyboardInput(m_onKeyReleased_SHIFT_id);
	m_inputSystem->UnregisterMouseButtonInput(m_onMouseButtonPressed_RIGHT_id);
	m_inputSystem->UnregisterMouseButtonInput(m_onMouseButtonReleased_RIGHT_id);
	m_inputSystem->UnregisterMouseCursorInput(m_onMouseMoved_id);

}
