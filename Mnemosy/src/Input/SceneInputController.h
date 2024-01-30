#pragma once
#include "Input/InputSystem.h"


class SceneInputController
{
public:
	SceneInputController(InputSystem& inputSystem,float& envrionmentRotation);
	~SceneInputController();


	void OnKeyPressed_LEFT_SHIFT(double deltaSeconds);
	void OnKeyReleased_LEFT_SHIFT(double deltaSeconds);

	void OnMouseButtonPressed_RIGHT(double deltaSeconds);
	void OnMouseButtonReleased_RIGHT(double deltaSeconds);

	void OnMouseMoved(double deltaSeconds, double xPos, double yPos, double xOffset, double yOffset);


private:

	void RegisterInputs();
	void UnregisterInputs();

private:
	float m_envrionmentRotationSpeed = 1;
	float* m_envrionmentRotation;
private:
	bool m_SHIFT_pressed = false;
	bool m_RMB_pressed = false;

	InputSystem* m_inputSystem;


	int m_onKeyPressed_SHIFT_id = -1;
	int m_onKeyReleased_SHIFT_id = -1;

	int m_onMouseButtonPressed_RIGHT_id = -1;
	int m_onMouseButtonReleased_RIGHT_id = -1;

	int m_onMouseMoved_id = -1;
};