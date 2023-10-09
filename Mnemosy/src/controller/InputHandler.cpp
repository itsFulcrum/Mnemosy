
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


	CURRENT_WINDOW_WIDTH = &currentWindowWidth;
	CURRENT_WINDOW_HEIGHT = &currentWindowHeight;
	
	float mouseLastX = float(*CURRENT_WINDOW_WIDTH * 0.5);
	float mouseLastY = float(*CURRENT_WINDOW_HEIGHT * 0.5);
	
	
	camera = mainCamera;

	this->time = &time;
	this->deltaTime = &deltaTime;
	this->lastFrame = &timeLastFrame;
}

void InputHandler::update(float &rotation)
{
	proccessKeyboardInput();

	ChangeEnvironmentRotation(rotation,mouseSensitivity);
}

void InputHandler::proccessKeyboardInput()
{
	// close window on escape
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

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
	*CURRENT_WINDOW_WIDTH =	 (unsigned int)width;
	*CURRENT_WINDOW_HEIGHT= (unsigned int)height;

	camera->updateScreenSize((unsigned int)width, (unsigned int)height);
	
}
void InputHandler::On_mouse_button_callback(int button, int action, int mods)
{
	// TODO: should stop moving if mouse doesnt move aswell
	SHIFT_AND_RMB_isPressed = false;

	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
	{
		if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
		{
			SHIFT_AND_RMB_isPressed = true;
		}
	}


	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE)
	{

		camera->cameraMode = CAMERA_MODE_FLY;



		SHIFT_AND_RMB_isPressed = false;
	}

	std::cout << "mousel btn callback: IsPredded = " << SHIFT_AND_RMB_isPressed << std::endl;

}
void InputHandler::On_mouse_cursor_callback(double xpos, double ypos)
{

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