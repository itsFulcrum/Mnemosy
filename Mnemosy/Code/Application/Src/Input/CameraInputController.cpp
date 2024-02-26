#include "Include/Input/CameraInputController.h"

#include "Include/Application.h"
#include "Include/Systems/Input/InputSystem.h"
#include "Include/Graphics/Camera.h"
#include "Include/Graphics/Scene.h"

#include <GLFW/glfw3.h>
#include <functional>

namespace mnemosy::input
{
	CameraInputController::CameraInputController()
	{
		m_cameraMode = CameraMode::CAMERA_MODE_FLY;

		RegisterInputs();
	}

	CameraInputController::~CameraInputController()
	{
		UnregisterInputs();
	}

	void CameraInputController::OnKeyPressed_W(double deltaSeconds)
	{
		if (!m_controllable) return;

		graphics::Camera& cam = ENGINE_INSTANCE().GetScene().GetCamera();

		float speed = float( m_settings.moveSpeed * deltaSeconds);

		glm::vec3 newPos = cam.transform.GetPosition();
		newPos += speed * cam.transform.GetForward();
		cam.transform.SetPosition(newPos);


	}

	void CameraInputController::OnKeyPressed_A(double deltaSeconds)
	{
		if (!m_controllable) return;

		graphics::Camera& cam = ENGINE_INSTANCE().GetScene().GetCamera();

		float speed = float(m_settings.moveSpeed * deltaSeconds);


		glm::vec3 newPos = cam.transform.GetPosition();
		newPos -= speed * cam.transform.GetRight();
		cam.transform.SetPosition(newPos);

	}

	void CameraInputController::OnKeyPressed_S(double deltaSeconds)
	{
		if (!m_controllable) return;

		graphics::Camera& cam = ENGINE_INSTANCE().GetScene().GetCamera();

		float speed = float(m_settings.moveSpeed * deltaSeconds);

		glm::vec3 newPos = cam.transform.GetPosition();
		newPos -= speed * cam.transform.GetForward();
		cam.transform.SetPosition(newPos);

	}

	void CameraInputController::OnKeyPressed_D(double deltaSeconds)
	{

		if (!m_controllable) return;

		graphics::Camera& cam = ENGINE_INSTANCE().GetScene().GetCamera();

		float speed = float(m_settings.moveSpeed * deltaSeconds);


		glm::vec3 newPos = cam.transform.GetPosition();
		newPos += speed * cam.transform.GetRight();
		cam.transform.SetPosition(newPos);
	}

	void CameraInputController::OnKeyPressed_ALT(double deltaSeconds)
	{
		//ENGINE_INSTANCE().GetInputSystem().HideCursor();
		m_controllable = true;
	}

	void CameraInputController::OnKeyReleased_ALT(double deltaSeconds)
	{
		//ENGINE_INSTANCE().GetInputSystem().UnlockCursor();
		m_controllable = false;
	}

	void CameraInputController::OnMouseScroll(double deltaSeconds, double xOffset, double yOffset)
	{
		if (!m_controllable) return;

		graphics::Camera& cam = ENGINE_INSTANCE().GetScene().GetCamera();

		if (m_cameraMode == CAMERA_MODE_EDIT)
		{



			glm::vec3 newPos = cam.transform.GetPosition();

			float f = float( m_settings.moveSpeed * yOffset);
			newPos *= cam.transform.GetForward() * f;
			//	m_camera->position *= m_camera->GetForward() * f;
			cam.transform.SetPosition(newPos);

			//if(yOffset > 0)
			//else if (yOffset < 0)
			//	position -= speed * forward;

		}

		if (m_cameraMode == CAMERA_MODE_FLY)
		{

			float fov = cam.settings.fov;
			fov = fov - (float)yOffset;
			fov = glm::clamp(fov, 1.0f, 80.0f);
			cam.settings.fov = fov;
		}
	}

	void CameraInputController::OnMouseMoved(double deltaSeconds, double xPos, double yPos, double xOffset, double yOffset)
	{

		if (!m_controllable) return;

		ENGINE_INSTANCE().GetInputSystem().HandleMouseOverViewport();
		graphics::Camera& cam = ENGINE_INSTANCE().GetScene().GetCamera();

		float mouseOffsetX = (float)xOffset;
		float mouseOffsetY = (float)yOffset;

		float angleX =  -(mouseOffsetY * m_settings.mouseSensitivity * (float)deltaSeconds);
		float angleY =  mouseOffsetX * m_settings.mouseSensitivity * (float)deltaSeconds;

		if (m_cameraMode == CAMERA_MODE_FLY)
		{
			cam.transform.RotateAroundAxis(angleX, cam.transform.GetRight());

			cam.transform.RotateAroundAxis(angleY, glm::vec3(0.0f,1.0f,0.0f));
		}
	}

	void CameraInputController::OnKeyPressed_ESCAPE(double deltaSeconds)
	{
		
		//ENGINE_INSTANCE().GetInputSystem().UnlockCursor();
	}

	void CameraInputController::OnMouseButtonPressed_LEFT(double deltaSeconds)
	{
		//ENGINE_INSTANCE().GetInputSystem().LockCursor();
	}

	void CameraInputController::OnScreenSizeChanged(int width, int height)
	{

		ENGINE_INSTANCE().GetScene().GetCamera().SetScreenSize(width, height);
	}

	// private
	void CameraInputController::RegisterInputs()
	{
		systems::InputSystem& inputSystem = ENGINE_INSTANCE().GetInputSystem();



		m_onKeyPressed_W_id		= inputSystem.REGISTER_KEYBOARD_INPUT(GLFW_KEY_W, GLFW_PRESS, false, &CameraInputController::OnKeyPressed_W);
		m_onKeyPressed_A_id		= inputSystem.REGISTER_KEYBOARD_INPUT(GLFW_KEY_A, GLFW_PRESS, false, &CameraInputController::OnKeyPressed_A);
		m_onKeyPressed_S_id		= inputSystem.REGISTER_KEYBOARD_INPUT(GLFW_KEY_S, GLFW_PRESS, false, &CameraInputController::OnKeyPressed_S);
		m_onKeyPressed_D_id		= inputSystem.REGISTER_KEYBOARD_INPUT(GLFW_KEY_D, GLFW_PRESS, false, &CameraInputController::OnKeyPressed_D);
		m_onKeyPressed_ALT_id	= inputSystem.REGISTER_KEYBOARD_INPUT(GLFW_KEY_LEFT_ALT, GLFW_PRESS, true,	&CameraInputController::OnKeyPressed_ALT);
		m_onKeyReleased_ALT_id	= inputSystem.REGISTER_KEYBOARD_INPUT(GLFW_KEY_LEFT_ALT, GLFW_RELEASE, true,&CameraInputController::OnKeyReleased_ALT);
		m_onMouseMoved_id		= inputSystem.REGISTER_MOUSE_CURSOR_INPUT(&CameraInputController::OnMouseMoved);
		m_onMouseScroll_id		= inputSystem.REGISTER_MOUSE_SCROLL_INPUT(&CameraInputController::OnMouseScroll);
		m_onScreenSizeChanged_id = inputSystem.REGISTER_WINDOW_RESIZE_INPUT(&CameraInputController::OnScreenSizeChanged);

		m_onKeyPressed_ESCAPE_id = inputSystem.REGISTER_KEYBOARD_INPUT(GLFW_KEY_ESCAPE, GLFW_PRESS, true, &CameraInputController::OnKeyPressed_ESCAPE);
		m_onMouseButtonPressed_LEFT_id = inputSystem.REGISTER_MOUSE_BUTTON_INPUT(GLFW_MOUSE_BUTTON_LEFT, GLFW_PRESS, &CameraInputController::OnMouseButtonPressed_LEFT);
	}

	void CameraInputController::UnregisterInputs()
	{
		systems::InputSystem& inputSystem = ENGINE_INSTANCE().GetInputSystem();

		inputSystem.UnregisterKeyboardInput(m_onKeyPressed_W_id);
		inputSystem.UnregisterKeyboardInput(m_onKeyPressed_A_id);
		inputSystem.UnregisterKeyboardInput(m_onKeyPressed_S_id);
		inputSystem.UnregisterKeyboardInput(m_onKeyPressed_D_id);
		inputSystem.UnregisterKeyboardInput(m_onKeyPressed_ALT_id);
		inputSystem.UnregisterKeyboardInput(m_onKeyReleased_ALT_id);
		inputSystem.UnregisterMouseCursorInput(m_onMouseMoved_id);
		inputSystem.UnregisterMouseScrollInput(m_onMouseScroll_id);
		inputSystem.UnregisterKeyboardInput(m_onKeyPressed_ESCAPE_id);
		inputSystem.UnregisterMouseButtonInput(m_onMouseButtonPressed_LEFT_id);
	}



} // mnemosy::input

