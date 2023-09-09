
#include "InputHandler.h"

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

void InputHandler::update()
{
	proccessKeyboardInput();
}

void InputHandler::proccessKeyboardInput()
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

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
	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
	{
		camera->cameraMode = CAMERA_MODE_EDIT;
	}
	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE)
	{
		camera->cameraMode = CAMERA_MODE_FLY;
	}
}
void InputHandler::On_mouse_cursor_callback(double xpos, double ypos)
{
	if (firstMouseInput)
	{
		mouseLastX = (float)xpos;
		mouseLastY = (float)ypos;
		firstMouseInput = false;
	}

	// called each time the mouse is moved
	float mouseOffsetX = (float)xpos - mouseLastX;
	float mouseOffsetY = mouseLastY - (float)ypos;
	mouseLastX = (float)xpos;
	mouseLastY = (float)ypos;


	camera->ProessMouseInput(mouseOffsetX, mouseOffsetY);
}
void InputHandler::On_mouse_scroll_callback(double offsetX, double offsetY)
{
	camera->ProcessMouseScrollInput((float)offsetY, *deltaTime);
}