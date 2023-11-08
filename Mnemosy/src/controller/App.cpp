

#include "App.h"


App::App()
{

	setupGLFWWindow("Mnemosy Texture library");
	

	// initialize glad "We pass GLAD the function to load the address of the OpenGL function pointers which is OS-specific"
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Warning!: Failed to initialize GLAD" << std::endl;
		glfwTerminate();
	}

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);


	userInterface = new GraphicalUserInterface(window);

	
	activeScene.Init(CURRENT_WINDOW_WIDTH, CURRENT_WINDOW_HEIGHT);
	
	inputHandler = new InputHandler();
	inputHandler->Init(window, CURRENT_WINDOW_WIDTH, CURRENT_WINDOW_HEIGHT, &activeScene.camera, time, deltaTime, lastFrame);
	glfwSetWindowUserPointer(window, inputHandler);



	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetScrollCallback(window, mouse_scroll_callback);
	glfwSetCursorPosCallback(window, mouse_cursor_callback);




	
}

App::~App()
{
	delete inputHandler;
	delete userInterface;

	glfwTerminate();
}


void App::setupGLFWWindow(const char* WindowTileName)
{
	// initialize glfw with opengl versions;
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
	glfwWindowHint(GLFW_SAMPLES, 4);

	// create GLFW window
	window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT,WindowTileName, nullptr, nullptr);





	if (window == nullptr)
	{
		std::cout << "Warning!: Failed to create GLFW window" << std::endl;
		glfwTerminate();
		//return-1;
	}
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1); // Enable vsync


}

void App::setupGlSettings()
{
	glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
	// number of attributes we can set in the shader supported by hardware
	bool Debug_PrintNumberOfAttributesToConsol = false;
	if (Debug_PrintNumberOfAttributesToConsol) 
	{
		int nrAttributes;
		glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &nrAttributes);
		std::cout << "Maximum nr of vertex attributes supported: " << nrAttributes << std::endl;
	}
	

	glEnable(GL_MULTISAMPLE);
	//enable depth testing
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

	// srgb is manually handled in the shaders
	glDisable(GL_FRAMEBUFFER_SRGB);

	// draw Polygons opaque (Fully shaded)

	glCullFace(GL_FRONT);
	glFrontFace(GL_CW);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	// wireframe mode
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
}



void App::run()
{
	ImGuiIO& io = ImGui::GetIO();

	setupGlSettings();


	//ModelLoader modelLoader;
	SceneRenderer sceneRenderer(SCR_WIDTH,SCR_HEIGHT);

	// ImGui Needs a refrence to the openGl Texture id so that we can later draw the renderTexture into the ImGui Image of the viewport Window
	userInterface->viewportRenderTextureID = sceneRenderer.GetRenderTextureId();
	
	

	glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);

	while (!glfwWindowShouldClose(window))
	{

		float time = static_cast<float>(glfwGetTime());
		deltaTime = time - lastFrame;
		lastFrame = time;



		float sinTime = sin(time)+1;


		// poll inputs from last frame
		glfwPollEvents();
		// proccess inputs
		if (userInterface->IsMouseOverViewport())
		{
			inputHandler->proccessInputs = true;
			inputHandler->update(activeScene.environmentRotation);
		}
		else
		{
			inputHandler->proccessInputs = false;
		}


		userInterface->UpdateWindowPosition(window);
		

		//  ImGui Needs to be rendered before OpenGl Scene
		userInterface->Render();



		
		activeScene.Update();
		



		sceneRenderer.RenderScene(&activeScene, userInterface->viewportWidth, userInterface->viewportHeight);


		glfwSwapBuffers(window);
	}
}


// CALLSBACKS
void framebuffer_size_callback(GLFWwindow* window, int width, int height) 
{
	//glViewport(0, 0, width, height);
	InputHandler* handler = (InputHandler*)glfwGetWindowUserPointer(window);
	handler->On_framebuffer_size_callback(width, height);
}

void mouse_cursor_callback(GLFWwindow* window, double posX, double posY)
{
	InputHandler* handler = (InputHandler*)glfwGetWindowUserPointer(window);
	handler->On_mouse_cursor_callback(posX, posY);


	
}
void mouse_button_callback(GLFWwindow* window,int button, int action, int mods)
{
	InputHandler* handler = (InputHandler*)glfwGetWindowUserPointer(window);
	handler->On_mouse_button_callback(button, action,mods);


	
}
void mouse_scroll_callback(GLFWwindow* window, double offsetX, double offsetY)
{
	InputHandler* handler = (InputHandler*)glfwGetWindowUserPointer(window);
	handler->On_mouse_scroll_callback(offsetX,offsetY);


}
