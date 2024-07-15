#include "Include/Input/SceneInputController.h"
#include "Include/MnemosyEngine.h"

#include "Include/Systems/Input/InputSystem.h"
#include "Include/Graphics/Skybox.h"
#include "Include/Graphics/Scene.h"
#include "Include/Graphics/Renderer.h"

#include <GLFW/glfw3.h>
#include <functional>

namespace mnemosy::input
{

	SceneInputController::SceneInputController()
	{
		RegisterInputs();
	}

	SceneInputController::~SceneInputController()
	{
		UnregisterInputs();
	}
	void SceneInputController::OnMouseMoved(double deltaSeconds, double xPos, double yPos, double xOffset, double yOffset)
	{
		//if (!m_SHIFT_pressed) return;

		if (!m_RMB_pressed) return;

		MnemosyEngine& engine = MnemosyEngine::GetInstance();

		engine.GetInputSystem().HandleMouseOverViewport();

		float amount = float(xOffset * m_envrionmentRotationSpeed * deltaSeconds);
		 //*m_envrionmentRotation += offset;
		engine.GetScene().GetSkybox().rotation += amount;

		engine.GetRenderer().SetShaderSkyboxUniforms(engine.GetScene().GetSkybox());
	}

	// private
	void SceneInputController::RegisterInputs()
	{
		systems::InputSystem& inputSystem = MnemosyEngine::GetInstance().GetInputSystem();

		
		//m_onKeyPressed_SHIFT_id = inputSystem.REGISTER_KEYBOARD_INPUT(GLFW_KEY_LEFT_SHIFT, GLFW_PRESS, true, &SceneInputController::OnKeyPressed_LEFT_SHIFT);
		//m_onKeyReleased_SHIFT_id = inputSystem.REGISTER_KEYBOARD_INPUT(GLFW_KEY_LEFT_SHIFT, GLFW_RELEASE, true, &SceneInputController::OnKeyReleased_LEFT_SHIFT);
		m_onMouseButtonPressed_RIGHT_id = inputSystem.REGISTER_MOUSE_BUTTON_INPUT(GLFW_MOUSE_BUTTON_RIGHT, GLFW_PRESS, &SceneInputController::OnMouseButtonPressed_RIGHT);
		m_onMouseButtonReleased_RIGHT_id = inputSystem.REGISTER_MOUSE_BUTTON_INPUT(GLFW_MOUSE_BUTTON_RIGHT, GLFW_RELEASE, &SceneInputController::OnMouseButtonReleased_RIGHT);
		m_onMouseMoved_id = inputSystem.REGISTER_MOUSE_CURSOR_INPUT(&SceneInputController::OnMouseMoved);
	}

	void SceneInputController::UnregisterInputs()
	{
		systems::InputSystem& inputSystem = MnemosyEngine::GetInstance().GetInputSystem();

		//inputSystem.UnregisterKeyboardInput(m_onKeyPressed_SHIFT_id);
		//inputSystem.UnregisterKeyboardInput(m_onKeyReleased_SHIFT_id);
		inputSystem.UnregisterMouseButtonInput(m_onMouseButtonPressed_RIGHT_id);
		inputSystem.UnregisterMouseButtonInput(m_onMouseButtonReleased_RIGHT_id);
		inputSystem.UnregisterMouseCursorInput(m_onMouseMoved_id);
	}

} // mnemosy::input