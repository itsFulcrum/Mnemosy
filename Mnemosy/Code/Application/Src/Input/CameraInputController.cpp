#include "Include/Input/CameraInputController.h"

//#include "Include/Application.h"
#include "Include/MnemosyEngine.h"
#include "Include/Core/Log.h"
#include "Include/Systems/Input/InputSystem.h"
#include "Include/Graphics/Camera.h"
#include "Include/Graphics/Scene.h"

#include <GLFW/glfw3.h>
#include <functional>

namespace mnemosy::input
{
	CameraInputController::CameraInputController()
		: m_camera{MnemosyEngine::GetInstance().GetScene().GetCamera()}
	{
		m_cameraMode = CameraMode::CAMERA_MODE_EDIT;

		RegisterInputs();

		// orient camera towards 0 point (look at the object)
		glm::vec3 camToZero = glm::normalize(-1.0f * m_camera.transform.GetPosition());
		glm::quat q = glm::quatLookAtLH(camToZero, m_worldUp);
		m_camera.transform.SetRotationQuaternion(q);

	}

	CameraInputController::~CameraInputController()
	{
		UnregisterInputs();
	}

	// not used at the moment
	/* // unused atm
	void CameraInputController::OnKeyPressed_W(double deltaSeconds)
	{
		if (!m_controllable) return;
		return;

		graphics::Camera& cam = ENGINE_INSTANCE().GetScene().GetCamera();

		float speed = float( m_settings.moveSpeed * deltaSeconds);

		glm::vec3 newPos = cam.transform.GetPosition();
		newPos += speed * cam.transform.GetForward();
		cam.transform.SetPosition(newPos);


	}

	void CameraInputController::OnKeyPressed_A(double deltaSeconds)
	{
		if (!m_controllable) return;
		return;

		graphics::Camera& cam = ENGINE_INSTANCE().GetScene().GetCamera();

		float speed = float(m_settings.moveSpeed * deltaSeconds);


		glm::vec3 newPos = cam.transform.GetPosition();
		newPos -= speed * cam.transform.GetRight();
		cam.transform.SetPosition(newPos);

	}

	void CameraInputController::OnKeyPressed_S(double deltaSeconds)
	{
		if (!m_controllable) return;
		return;


		graphics::Camera& cam = ENGINE_INSTANCE().GetScene().GetCamera();

		float speed = float(m_settings.moveSpeed * deltaSeconds);

		glm::vec3 newPos = cam.transform.GetPosition();
		newPos -= speed * cam.transform.GetForward();
		cam.transform.SetPosition(newPos);

	}

	void CameraInputController::OnKeyPressed_D(double deltaSeconds)
	{

		if (!m_controllable) return;
		return;


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
	*/

	void CameraInputController::OnMouseScroll(double deltaSeconds, double xOffset, double yOffset) {
		//if (!m_controllable) return; // always allow scrol / zoom


		glm::vec3 currentPos = m_camera.transform.GetPosition();


		float forwardAmount = float(m_settings.moveSpeed * yOffset);
		glm::vec3 newPos = currentPos + (m_camera.transform.GetForward() * forwardAmount);
		// distance to 0 point 
		float distance = glm::distance(newPos, glm::vec3(0.0f, 0.0f, 0.0f));

		if (distance <= 20.0f) {

			// calculate a 0-1 value withing range 0-16 and use it to scale move speed so the closer you get to the object the slower the camera is.
			float distanceNormalized = distance / 20.0f;
			forwardAmount *= distanceNormalized;

			newPos = currentPos + (m_camera.transform.GetForward() * forwardAmount);
			
			// recalculate distance for the new postion to have a precize hard lock check at the end
			distance = glm::distance(newPos, glm::vec3(0.0f, 0.0f, 0.0f));

		}
		else if (distance > 75.0f) // hard lock when to far
		{
			return;
		}

		if (distance < 0.6f) // hard lock when too close
			return;


		m_camera.transform.SetPosition(newPos);
	}

	void CameraInputController::OnMouseMoved(double deltaSeconds, double xPos, double yPos, double xOffset, double yOffset)
	{

		if (!m_controllable) return;

		MnemosyEngine::GetInstance().GetInputSystem().HandleMouseOverViewport();


		if (m_cameraMode == CAMERA_MODE_EDIT) {

			glm::vec3 camPos = m_camera.transform.GetPosition();

		
			//  calc new position rotated around global y axis
			float angleY = xOffset * m_settings.mouseSensitivity * deltaSeconds;
			glm::quat horizontalRot = glm::angleAxis(-angleY,m_worldUp);
			glm::vec3 newPos = horizontalRot * camPos;


			// calc new position rotated around local right vector
			float angleX = yOffset * m_settings.mouseSensitivity * deltaSeconds * 0.5f;
			glm::quat verticalRot = glm::angleAxis(angleX, m_camera.transform.GetRight());
			glm::vec3 newVerticalPos = verticalRot * newPos;


			// apply horizontal movement only up to a given degree
			float dot = glm::dot(glm::normalize(newVerticalPos), m_worldUp);
			if (dot < 0.97f && dot > -0.97f) {
				newPos = newVerticalPos;
			}

			m_camera.transform.SetPosition(newPos);

			// always orient camera towards 0 point
			glm::vec3 camToZero = glm::normalize(-1.0f * m_camera.transform.GetPosition());
			glm::quat q = glm::quatLookAtLH(camToZero, m_worldUp);
			m_camera.transform.SetRotationQuaternion(q);
		}



		//else if (m_cameraMode == CAMERA_MODE_FLY) {


		//	float mouseOffsetX = (float)xOffset;
		//	float mouseOffsetY = (float)yOffset;

		//	float angleX =  -(mouseOffsetY * m_settings.mouseSensitivity * (float)deltaSeconds);
		//	float angleY =  mouseOffsetX * m_settings.mouseSensitivity * (float)deltaSeconds;

		//	m_camera.transform.RotateAroundAxis(angleX, m_camera.transform.GetRight());
		//	m_camera.transform.RotateAroundAxis(angleY, glm::vec3(0.0f,1.0f,0.0f));
		//
		//}
	}

	void CameraInputController::OnMouseButtonPressed_LEFT(double deltaSeconds)
	{
		m_controllable = true;
	}

	void CameraInputController::OnMouseButtonReleased_LEFT(double deltaSeconds)
	{
		m_controllable = false;
	}

	void CameraInputController::OnScreenSizeChanged(int width, int height)
	{

		MnemosyEngine::GetInstance().GetScene().GetCamera().SetScreenSize(width, height);
	}

	// private
	void CameraInputController::RegisterInputs()
	{
		systems::InputSystem& inputSystem = MnemosyEngine::GetInstance().GetInputSystem();



		//m_onKeyPressed_W_id		= inputSystem.REGISTER_KEYBOARD_INPUT(GLFW_KEY_W, GLFW_PRESS, false, &CameraInputController::OnKeyPressed_W);
		//m_onKeyPressed_A_id		= inputSystem.REGISTER_KEYBOARD_INPUT(GLFW_KEY_A, GLFW_PRESS, false, &CameraInputController::OnKeyPressed_A);
		//m_onKeyPressed_S_id		= inputSystem.REGISTER_KEYBOARD_INPUT(GLFW_KEY_S, GLFW_PRESS, false, &CameraInputController::OnKeyPressed_S);
		//m_onKeyPressed_D_id		= inputSystem.REGISTER_KEYBOARD_INPUT(GLFW_KEY_D, GLFW_PRESS, false, &CameraInputController::OnKeyPressed_D);
		//m_onKeyPressed_ALT_id	= inputSystem.REGISTER_KEYBOARD_INPUT(GLFW_KEY_LEFT_ALT, GLFW_PRESS, true,	&CameraInputController::OnKeyPressed_ALT);
		//m_onKeyReleased_ALT_id	= inputSystem.REGISTER_KEYBOARD_INPUT(GLFW_KEY_LEFT_ALT, GLFW_RELEASE, true,&CameraInputController::OnKeyReleased_ALT);


		m_onMouseMoved_id		= inputSystem.REGISTER_MOUSE_CURSOR_INPUT(&CameraInputController::OnMouseMoved);
		m_onMouseScroll_id		= inputSystem.REGISTER_MOUSE_SCROLL_INPUT(&CameraInputController::OnMouseScroll);
		m_onScreenSizeChanged_id = inputSystem.REGISTER_WINDOW_RESIZE_INPUT(&CameraInputController::OnScreenSizeChanged);



		//m_onKeyPressed_ESCAPE_id = inputSystem.REGISTER_KEYBOARD_INPUT(GLFW_KEY_ESCAPE, GLFW_PRESS, true, &CameraInputController::OnKeyPressed_ESCAPE);
		m_onMouseButtonPressed_LEFT_id = inputSystem.REGISTER_MOUSE_BUTTON_INPUT(GLFW_MOUSE_BUTTON_LEFT, GLFW_PRESS, &CameraInputController::OnMouseButtonPressed_LEFT);
		m_onMouseButtonReleased_LEFT_id = inputSystem.REGISTER_MOUSE_BUTTON_INPUT(GLFW_MOUSE_BUTTON_LEFT, GLFW_RELEASE, &CameraInputController::OnMouseButtonReleased_LEFT);
	}

	void CameraInputController::UnregisterInputs()
	{
		systems::InputSystem& inputSystem = MnemosyEngine::GetInstance().GetInputSystem();

		//inputSystem.UnregisterKeyboardInput(m_onKeyPressed_W_id);
		//inputSystem.UnregisterKeyboardInput(m_onKeyPressed_A_id);
		//inputSystem.UnregisterKeyboardInput(m_onKeyPressed_S_id);
		//inputSystem.UnregisterKeyboardInput(m_onKeyPressed_D_id);
		//inputSystem.UnregisterKeyboardInput(m_onKeyPressed_ALT_id);
		//inputSystem.UnregisterKeyboardInput(m_onKeyReleased_ALT_id);
		inputSystem.UnregisterMouseCursorInput(m_onMouseMoved_id);
		inputSystem.UnregisterMouseScrollInput(m_onMouseScroll_id);
		//inputSystem.UnregisterKeyboardInput(m_onKeyPressed_ESCAPE_id);
		inputSystem.UnregisterMouseButtonInput(m_onMouseButtonPressed_LEFT_id);
		inputSystem.UnregisterMouseButtonInput(m_onMouseButtonReleased_LEFT_id);
	}



} // mnemosy::input

