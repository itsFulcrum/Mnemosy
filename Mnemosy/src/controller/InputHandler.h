#ifndef INPUT_HANDLER_H
#define INPUT_HANDLER_H


#include <GLFW/glfw3.h>

#include <model/Camera.h>


class InputHandler
{
public:
	InputHandler();
	~InputHandler();

	void Init(GLFWwindow* window,unsigned int& currentWindowWidth, unsigned int& currentWindowHeight, Camera* camera, float& time, float& deltaTime, float& timeLastFrame);

	// every frame
	void update(float &rotation);
	void proccessKeyboardInput();

	void On_framebuffer_size_callback(int width, int height);
	void On_mouse_button_callback(int button, int action, int mods);
	void On_mouse_cursor_callback(double xpos, double ypos);
	void On_mouse_scroll_callback(double offsetX, double offsetY);

	//
	void OnKeyPressed_ALT();
	void OnKeyReleased_ALT();
	void OnKeyPressed_SHIFT();
	void OnKeyReleased_SHIFT();


	// functions
	void ChangeEnvironmentRotation(float &rotation, float sensitivity);
	void HandleMouseOverViewport(
		int mouseScreenPosX, int mouseScreenPosY,
		int viewportStartScreenPosX, int viewportStartScreenPosY,
		int viewportWidth, int viewportHeight,
		int mainWindowStartScreenPosX, int mainWindowStartScreenPosY);


	//
	// disabled if im gui wants to use inputs
	bool proccessInputs = true;


	// Settings
	float mouseSensitivity = 0.1;

	// mouse
	float mouseCurrentX = 0.0;
	float mouseCurrentY = 0.0;
	float mouseLastX = 0.0;
	float mouseLastY = 0.0;
	float mouseOffsetX = 0.0;
	float mouseOffsetY = 0.0;

	bool firstMouseInput = true;

	bool ALT_isPressed = false;
	bool SHIFT_isPressed = false;
	bool SHIFT_AND_RMB_isPressed = false;

	bool mouseLocked = false;
	//bool ALT_isPressed = false;

	// pointers to the variables used by the app class
	//  need to be setup in the init funciton
	float* time = nullptr;
	float* deltaTime = nullptr;
	float* lastFrame = nullptr;


	unsigned int* CURRENT_WINDOW_WIDTH = nullptr;
	unsigned int* CURRENT_WINDOW_HEIGHT = nullptr;

	Camera* camera = nullptr;
	GLFWwindow* window;
};



#endif // !INPUT_HANDLER_H