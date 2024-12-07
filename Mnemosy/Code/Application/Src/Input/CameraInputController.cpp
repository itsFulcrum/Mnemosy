#include "Include/Input/CameraInputController.h"

#include "Include/MnemosyEngine.h"
//#include "Include/Core/Log.h"
#include "Include/Systems/Input/InputSystem.h"

#include "Include/Graphics/Camera.h"
#include "Include/Graphics/Renderer.h"
#include "Include/Graphics/Scene.h"
#include "Include/Graphics/Skybox.h"

#include <glm/glm.hpp>
#include <GLFW/glfw3.h>
#include <functional>

namespace mnemosy::input {

	void CameraInputController::Init(){

		m_LMB_pressed = false;
		m_RMB_pressed = false;
		m_camMoveSpeed 	= 1.0f;
		m_camRotSpeed 	= 1.1f;
		m_skyboxRotSpeed = 1.25f;

		// Register inputs
		systems::InputSystem& inputSystem = MnemosyEngine::GetInstance().GetInputSystem();


		inputSystem.REGISTER_MOUSE_CURSOR_INPUT(&CameraInputController::OnMouseMoved);
		inputSystem.REGISTER_MOUSE_SCROLL_INPUT(&CameraInputController::OnMouseScroll);
		inputSystem.REGISTER_WINDOW_RESIZE_INPUT(&CameraInputController::OnScreenSizeChanged);


		inputSystem.REGISTER_MOUSE_BUTTON_INPUT(GLFW_MOUSE_BUTTON_LEFT, GLFW_PRESS, &CameraInputController::OnMouseButtonPressed_LEFT);
		inputSystem.REGISTER_MOUSE_BUTTON_INPUT(GLFW_MOUSE_BUTTON_LEFT, GLFW_RELEASE, &CameraInputController::OnMouseButtonReleased_LEFT);
	

		inputSystem.REGISTER_MOUSE_BUTTON_INPUT(GLFW_MOUSE_BUTTON_RIGHT, GLFW_PRESS, &CameraInputController::OnMouseButtonPressed_RIGHT);
		inputSystem.REGISTER_MOUSE_BUTTON_INPUT(GLFW_MOUSE_BUTTON_RIGHT, GLFW_RELEASE, &CameraInputController::OnMouseButtonReleased_RIGHT);
	

		graphics::Camera& cam = MnemosyEngine::GetInstance().GetScene().GetCamera();

		// setup and orient camera towards 0 point (looking at the object)
		glm::vec3 camToZero = glm::normalize(-1.0f * cam.transform.GetPosition());
		glm::quat q = glm::quatLookAtLH(camToZero, glm::vec3(0.0f, 1.0f, 0.0f));
		cam.transform.SetRotationQuaternion(q);
	}


	void CameraInputController::OnMouseScroll(double deltaSeconds, double xOffset, double yOffset) {
		
		// moves camera forward and backward	
		
		graphics::Camera& cam = MnemosyEngine::GetInstance().GetScene().GetCamera();

		glm::vec3 currentPos = cam.transform.GetPosition();

		float forwardAmount = float(m_camMoveSpeed * yOffset);
		glm::vec3 newPos = currentPos + (cam.transform.GetForward() * forwardAmount);
		// distance to 0 point 
		float distance = glm::distance(newPos, glm::vec3(0.0f, 0.0f, 0.0f));

		if (distance <= 20.0f) {

			// calculate a 0-1 value withing range 0-16 and use it to scale move speed so the closer you get to the object the slower the camera is.
			float distanceNormalized = distance / 20.0f;
			forwardAmount *= distanceNormalized;

			newPos = currentPos + (cam.transform.GetForward() * forwardAmount);
			
			// recalculate distance for the new postion to have a precize hard lock check at the end
			distance = glm::distance(newPos, glm::vec3(0.0f, 0.0f, 0.0f));

		}
		else if (distance > 75.0f)  { // hard lock when to far
			return;
		}

		if (distance < 0.6f) { // hard lock when too close
			return;
		}

		cam.transform.SetPosition(newPos);
	}

	void CameraInputController::OnMouseMoved(double deltaSeconds, double xPos, double yPos, double xOffset, double yOffset) {

		systems::InputSystem& inputSystem = MnemosyEngine::GetInstance().GetInputSystem();

		if(!inputSystem.IsMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT)){
				m_LMB_pressed = false;
		}

		if(m_LMB_pressed || m_RMB_pressed){
			inputSystem.HandleMouseOverViewport();
		}


		if(m_LMB_pressed) {
			RotateCamera(deltaSeconds,xPos,yPos,xOffset,yOffset);
		}

		if(m_RMB_pressed){
			RotateSkybox(deltaSeconds,xPos,yPos,xOffset,yOffset);
		}
	}


	void CameraInputController::OnScreenSizeChanged(int width, int height) {

		MnemosyEngine::GetInstance().GetScene().GetCamera().SetScreenSize(width, height);
	}

	// private

	void CameraInputController::RotateCamera(double deltaSeconds, double xPos, double yPos, double xOffset, double yOffset){

		graphics::Camera& cam = MnemosyEngine::GetInstance().GetScene().GetCamera();

		glm::vec3 camPos = cam.transform.GetPosition();


		glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
	
		//  calc new position rotated around global y axis
		float angleY = xOffset * m_camRotSpeed * deltaSeconds;
		glm::quat horizontalRot = glm::angleAxis(-angleY,worldUp);
		glm::vec3 newPos = horizontalRot * camPos;


		// calc new position rotated around local right vector
		float angleX = yOffset * m_camRotSpeed * deltaSeconds * 0.5f;
		glm::quat verticalRot = glm::angleAxis(angleX, cam.transform.GetRight());
		glm::vec3 newVerticalPos = verticalRot * newPos;


		// apply horizontal movement only up to a given degree
		float dot = glm::dot(glm::normalize(newVerticalPos), worldUp);
		if (dot < 0.97f && dot > -0.97f) {
			newPos = newVerticalPos;
		}

		cam.transform.SetPosition(newPos);

		// always orient camera towards 0 point
		glm::vec3 camToZero = glm::normalize(-1.0f * cam.transform.GetPosition());
		glm::quat q = glm::quatLookAtLH(camToZero, worldUp);
		cam.transform.SetRotationQuaternion(q);
	}

	void CameraInputController::RotateSkybox(double deltaSeconds, double xPos, double yPos, double xOffset, double yOffset){

		MnemosyEngine& engine = MnemosyEngine::GetInstance();

		engine.GetScene().userSceneSettings.background_rotation += (float)(xOffset * m_skyboxRotSpeed * deltaSeconds);

		engine.GetRenderer().SetShaderSkyboxUniforms(engine.GetScene().userSceneSettings, engine.GetScene().GetSkybox());

	}


} // mnemosy::input

