
#include "InputHandler.h"
#include "iostream"

InputHandler::InputHandler()
{
	CURRENT_WINDOW_WIDTH = nullptr;
	CURRENT_WINDOW_HEIGHT = nullptr;

	camera = nullptr;

	time = nullptr;
	deltaTime = nullptr;
	lastFrame = nullptr;

	window = nullptr;
}

InputHandler::~InputHandler()
{
	// deletion causes crash 
	// 
	//delete CURRENT_WINDOW_HEIGHT;
	//delete CURRENT_WINDOW_WIDTH;
	//delete camera;
	//delete time;
	//delete deltaTime;
	//delete lastFrame;
	//delete window;
	
}



void InputHandler::Init(GLFWwindow* window,unsigned int& currentWindowWidth, unsigned int& currentWindowHeight, Camera* mainCamera, float& time,float& deltaTime,float& timeLastFrame)
{
	
	this->window = window;
	this->camera = mainCamera;


	CURRENT_WINDOW_WIDTH = &currentWindowWidth;
	CURRENT_WINDOW_HEIGHT = &currentWindowHeight;
	
	float mouseLastX = float(*CURRENT_WINDOW_WIDTH * 0.5);
	float mouseLastY = float(*CURRENT_WINDOW_HEIGHT * 0.5);
	
	

	this->time = &time;
	this->deltaTime = &deltaTime;
	this->lastFrame = &timeLastFrame;
}

void InputHandler::update(float &rotation)
{
	if (!proccessInputs)
		return;


	proccessKeyboardInput();
	ChangeEnvironmentRotation(rotation,mouseSensitivity);
}

void InputHandler::proccessKeyboardInput()
{
	// close window on escape
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	// == ALT ==
	if (glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS)
	{
		if (!ALT_isPressed)
		{
			ALT_isPressed = true;
			OnKeyPressed_ALT();
		}
	}
	if (glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_RELEASE)
	{
		if (ALT_isPressed) 
		{
			ALT_isPressed = false;
			OnKeyReleased_ALT();
		}
	}
	// == SHIFT ==
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
	{
		if (!SHIFT_isPressed)
		{
			SHIFT_isPressed = true;
			OnKeyPressed_SHIFT();
		}
	}
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_RELEASE)
	{
		if (SHIFT_isPressed)
		{
			SHIFT_isPressed = false;
			SHIFT_AND_RMB_isPressed = false;
			OnKeyReleased_SHIFT();
			
		}
	}

	// camera WASD Movement
	if (camera == nullptr)
		return;

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
	{
		camera->ProcessKeyboardInput(CAMERA_FORWARD, *deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
	{
		camera->ProcessKeyboardInput(CAMERA_LEFT, *deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
	{
		camera->ProcessKeyboardInput(CAMERA_BACKWARD, *deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
	{
		camera->ProcessKeyboardInput(CAMERA_RIGHT, *deltaTime);
	}
}


void InputHandler::On_framebuffer_size_callback(int width, int height)
{
	if (!proccessInputs)
		return;

	*CURRENT_WINDOW_WIDTH =	 (unsigned int)width;
	*CURRENT_WINDOW_HEIGHT= (unsigned int)height;

	camera->updateScreenSize((unsigned int)width, (unsigned int)height);
	
}
void InputHandler::On_mouse_button_callback(int button, int action, int mods)
{
	if (!proccessInputs)
		return;

	std::cout << "Receive inputs" << std::endl;
	// TODO: should stop moving if mouse doesnt move aswell
	//SHIFT_AND_RMB_isPressed = false;

	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
	{
		if (SHIFT_isPressed)
		{
			SHIFT_AND_RMB_isPressed = true;
		}
	}
	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE)
	{
		camera->cameraMode = CAMERA_MODE_FLY;
		SHIFT_AND_RMB_isPressed = false;
	}
	// Left Mouse Button
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
	{

	}
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
	{

	}

}
void InputHandler::On_mouse_cursor_callback(double xpos, double ypos)
{
	if (!proccessInputs)
		return;

	mouseCurrentX = xpos;
	mouseCurrentY = ypos;

	if (firstMouseInput)
	{
		mouseLastX = (float)xpos;
		mouseLastY = (float)ypos;
		firstMouseInput = false;
	}

	// called each time the mouse is moved
	 mouseOffsetX = (float)xpos - mouseLastX;
	 mouseOffsetY = mouseLastY - (float)ypos;


	mouseLastX = (float)xpos;
	mouseLastY = (float)ypos;

	if(!SHIFT_AND_RMB_isPressed)
		camera->ProessMouseInput(mouseOffsetX, mouseOffsetY);


}
void InputHandler::On_mouse_scroll_callback(double offsetX, double offsetY)
{
	if (!proccessInputs)
		return;

	camera->ProcessMouseScrollInput((float)offsetY, *deltaTime);
}


void InputHandler::ChangeEnvironmentRotation(float &rotation,float sensitivity)
{
	if (!SHIFT_AND_RMB_isPressed)
		return;

	//float a = mouseCurrentX - mouseLastX;
	//std::cout << "input handler: a: " << mouseOffsetX << std::endl;
	// TODO: should stop moving if mouse doesnt move aswell
	rotation += mouseOffsetX * sensitivity * *deltaTime;
}

void InputHandler::HandleMouseOverViewport(
	int mouseScreenPosX, int mouseScreenPosY, 
	int viewportStartScreenPosX, int viewportStartScreenPosY, 
	int viewportWidth, int viewportHeight, 
	int mainWindowStartScreenPosX, int mainWindowStartScreenPosY)
{
	if (!mouseLocked)
		return;

	// glfw Sets the mouse courser position based on numbers relative to the upper left corner (0,0) of the main window area


	
	
	if (mouseScreenPosX < viewportStartScreenPosX)
	{
		// set mouse pos x to windowStartPos.x + windowWidth

		double newXpos = (viewportStartScreenPosX + viewportWidth) - mainWindowStartScreenPosX;

		glfwSetCursorPos(window, newXpos, (double)mouseScreenPosY - mainWindowStartScreenPosY);
	}
	if (mouseScreenPosX > (viewportStartScreenPosX + viewportWidth))
	{
		// set mouse pos x to viewportStartPos.x
		double newXpos = viewportStartScreenPosX - mainWindowStartScreenPosX;
		glfwSetCursorPos(window, newXpos, (double)mouseScreenPosY - mainWindowStartScreenPosY);

	}


	if (mouseScreenPosY < viewportStartScreenPosY)
	{
		// set mouse pos y to windowStartPos.y + windowheight
		double newYPos = viewportStartScreenPosY + viewportHeight - mainWindowStartScreenPosY;

		glfwSetCursorPos(window, (double)mouseScreenPosX - mainWindowStartScreenPosX, newYPos);
	}
	if (mouseScreenPosY > (viewportStartScreenPosY + viewportHeight))
	{
		// set mouse pos y to windowStartPos.y
		double newYPos = viewportStartScreenPosY - mainWindowStartScreenPosY;
		glfwSetCursorPos(window, (double)mouseScreenPosX - mainWindowStartScreenPosX, newYPos);
	}
}

void InputHandler::OnKeyPressed_ALT()
{
	camera->ActivateControlls(window);
	mouseLocked = true;
	//glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}
void InputHandler::OnKeyReleased_ALT()
{
	camera->DeactivateControlls(window);
	mouseLocked = false;
	//glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}
void InputHandler::OnKeyPressed_SHIFT()
{
	//glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	mouseLocked = true;
	if(ALT_isPressed)
		camera->DeactivateControlls(window);
}
void InputHandler::OnKeyReleased_SHIFT()
{
	//glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	mouseLocked = false;
	if(ALT_isPressed)
		camera->ActivateControlls(window);
}