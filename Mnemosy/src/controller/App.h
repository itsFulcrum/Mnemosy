
#ifndef APPLICATION_H
#define APPLICATION_H


#include <glad/glad.h>
#include <GLFW/glfw3.h>


#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>



// internal
#include <config.h>

#include <model/Camera.h>
#include <model/Model.h>
#include <model/Object.h>

#include <view/Shader.h>
#include <view/Texture.h>
#include <view/Cubemap.h>
#include <view/PbrMaterial.h>

#include <controller/InputHandler.h>



void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void mouse_cursor_callback(GLFWwindow* window, double xpos, double ypos);
void mouse_scroll_callback(GLFWwindow* window, double offsetX, double offsetY);


class App
{
public:
	App();
	~App();
	void run();


	//Settings
	unsigned int SCR_WIDTH = 1024;
	unsigned int SCR_HEIGHT = 800;

	unsigned int CURRENT_WINDOW_WIDTH = SCR_WIDTH;
	unsigned int CURRENT_WINDOW_HEIGHT = SCR_HEIGHT;



	float time = 0.0;
	float deltaTime = 0.0;
	float lastFrame = 0.0;

private:
	void setupGlSettings();
	// this will eventually be move into its own class
	void setupScene();

	GLFWwindow* window;

	Camera* mainCamera;
	InputHandler* inputHandler;
	
};



#endif // !APPLICATION_H