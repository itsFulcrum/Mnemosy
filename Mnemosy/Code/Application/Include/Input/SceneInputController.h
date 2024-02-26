#ifndef SCENE_INPUT_CONTROLLER_H
#define SCENE_INPUT_CONTROLLER_H

namespace mnemosy::input
{
	class SceneInputController
	{
	public:
		SceneInputController();
		~SceneInputController();


		void OnKeyPressed_LEFT_SHIFT(double deltaSeconds)		{ m_SHIFT_pressed = true; }
		void OnKeyReleased_LEFT_SHIFT(double deltaSeconds)		{ m_SHIFT_pressed = false; }

		void OnMouseButtonPressed_RIGHT(double deltaSeconds)	{ m_RMB_pressed = true; }
		void OnMouseButtonReleased_RIGHT(double deltaSeconds)	{ m_RMB_pressed = false; }

		void OnMouseMoved(double deltaSeconds, double xPos, double yPos, double xOffset, double yOffset);

	private:

		void RegisterInputs();
		void UnregisterInputs();

	private:
		float m_envrionmentRotationSpeed = 1.5f;
	private:
		bool m_SHIFT_pressed = false;
		bool m_RMB_pressed = false;


		int m_onKeyPressed_SHIFT_id = -1;
		int m_onKeyReleased_SHIFT_id = -1;

		int m_onMouseButtonPressed_RIGHT_id = -1;
		int m_onMouseButtonReleased_RIGHT_id = -1;

		int m_onMouseMoved_id = -1;
	};
}

#endif // !SCENE_INPUT_CONTROLLER_H
