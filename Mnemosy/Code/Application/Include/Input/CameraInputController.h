#ifndef CAMERA_CONTROLLER_H
#define CAMERA_CONTROLLER_H

namespace mnemosy::input {

	class CameraInputController
	{
	public:
		CameraInputController() = default;
		~CameraInputController() = default;


		void Init();

		void OnMouseScroll(double deltaSeconds, double xOffset, double yOffset);
		void OnMouseMoved(double deltaSeconds, double xPos, double yPos, double xOffset, double yOffset);

		void OnScreenSizeChanged(int width, int height);


		void OnMouseButtonPressed_LEFT (double deltaSeconds)  { m_LMB_pressed = true; }
		void OnMouseButtonReleased_LEFT(double deltaSeconds)  { m_LMB_pressed = false;}

		void OnMouseButtonPressed_RIGHT (double deltaSeconds) { m_RMB_pressed = true; }
		void OnMouseButtonReleased_RIGHT(double deltaSeconds) { m_RMB_pressed = false;}

	private:		
		void RotateCamera(double deltaSeconds, double xPos, double yPos, double xOffset, double yOffset);
		void RotateSkybox(double deltaSeconds, double xPos, double yPos, double xOffset, double yOffset);

		bool m_LMB_pressed = false;
		bool m_RMB_pressed = false;
		float m_camMoveSpeed 	= 1.0f;
		float m_camRotSpeed 	= 1.1f;
		float m_skyboxRotSpeed 	= 1.25f;
	};

}
#endif // !CAMERA_CONTROLLER_H
