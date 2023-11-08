
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
#include <model/Object.h>

#include <view/Shader.h>
#include <view/Texture.h>
#include <view/Cubemap.h>
#include <view/PbrMaterial.h>

#include <view/GraphicalUserInterface.h>


#include <controller/InputHandler.h>
#include <controller/SceneRenderer.h>
#include <controller/ModelLoader.h>

//#include <view/scene/Scene.h>
#include <view/scene/DefaultScene.h>


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

private:
	void setupGlSettings();
	void setupGLFWWindow(const char* WindowTitleName);

public:
	//Settings
	unsigned int SCR_WIDTH = 1024;
	unsigned int SCR_HEIGHT = 800;

	unsigned int CURRENT_WINDOW_WIDTH = SCR_WIDTH;
	unsigned int CURRENT_WINDOW_HEIGHT = SCR_HEIGHT;



	float time = 0.0;
	float deltaTime = 0.0;
	float lastFrame = 0.0;

private:
	GLFWwindow* window;
	InputHandler* inputHandler;
	GraphicalUserInterface* userInterface;
	

	DefaultScene activeScene;
};



#endif // !APPLICATION_H