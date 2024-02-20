#ifndef CAMERA_CONTROLLER_H
#define CAMERA_CONTROLLER_H

#include <glm/glm.hpp>

namespace mnemosy::input
{
	enum CameraMode
	{
		CAMERA_MODE_EDIT,
		CAMERA_MODE_FLY
	};

	struct CameraControllerSettings
	{
		float moveSpeed = 6;
		float mouseSensitivity = 18;
	};

	class CameraInputController
	{
	public:
		CameraInputController();
		~CameraInputController();

		void OnKeyPressed_W(double deltaSeconds);
		void OnKeyPressed_A(double deltaSeconds);
		void OnKeyPressed_S(double deltaSeconds);
		void OnKeyPressed_D(double deltaSeconds);

		void OnKeyPressed_ALT(double deltaSeconds);
		void OnKeyReleased_ALT(double deltaSeconds);


		void OnMouseScroll(double deltaSeconds, double xOffset, double yOffset);
		void OnMouseMoved(double deltaSeconds, double xPos, double yPos, double xOffset, double yOffset);

		void OnKeyPressed_ESCAPE(double deltaSeconds);
		void OnMouseButtonPressed_LEFT(double deltaSeconds);


		void OnScreenSizeChanged(int width, int height);

	private:
		void RegisterInputs();
		void UnregisterInputs();

		bool m_controllable = false;
		//glm::vec3 focusPoint = glm::vec3(0.0f, 0.0f, 0.0f);

		CameraControllerSettings m_settings;
		CameraMode m_cameraMode;
		
		int m_onKeyPressed_W_id = -1;
		int m_onKeyPressed_A_id = -1;
		int m_onKeyPressed_S_id = -1;
		int m_onKeyPressed_D_id = -1;

		int m_onKeyPressed_ALT_id = -1;
		int m_onKeyReleased_ALT_id = -1;
		int m_onMouseScroll_id = -1;
		int m_onMouseMoved_id = -1;

		int m_onKeyPressed_ESCAPE_id = -1;
		int m_onMouseButtonPressed_LEFT_id = -1;

		int m_onScreenSizeChanged_id = -1;

	};

}
#endif // !CAMERA_CONTROLLER_H
