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
	void update();

	void proccessKeyboardInput();

	void On_framebuffer_size_callback(int width, int height);
	void On_mouse_button_callback(int button, int action, int mods);
	void On_mouse_cursor_callback(double xpos, double ypos);
	void On_mouse_scroll_callback(double offsetX, double offsetY);



	// mouse
	float mouseLastX = 0.0;
	float mouseLastY = 0.0;
	bool firstMouseInput = true;

	// pointers to the variables used by the app class
	//  need to be setup in the init funciton
	float* time = nullptr;
	float* deltaTime = nullptr;
	float* lastFrame = nullptr;


	unsigned int* CURRENT_WINDOW_WIDTH = nullptr;
	unsigned int* CURRENT_WINDOW_HEIGHT = nullptr;

	Camera* camera = nullptr;
	GLFWwindow* window = nullptr;
};



#endif // !INPUT_HANDLER_H