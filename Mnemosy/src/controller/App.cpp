

#include "App.h"
void APIENTRY glDebugOutput(GLenum source, GLenum type, unsigned int id, GLenum severity,
	GLsizei length, const char* message, const void* userParam)
{
	// ignore non-significant error/warning codes
	if (id == 131169 || id == 131185 || id == 131218 || id == 131204) return;

	std::cout << "---------------" << std::endl;
	std::cout << "Debug message (" << id << "): " << message << std::endl;

	switch (source)
	{
	case GL_DEBUG_SOURCE_API:             std::cout << "Source: API"; break;
	case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   std::cout << "Source: Window System"; break;
	case GL_DEBUG_SOURCE_SHADER_COMPILER: std::cout << "Source: Shader Compiler"; break;
	case GL_DEBUG_SOURCE_THIRD_PARTY:     std::cout << "Source: Third Party"; break;
	case GL_DEBUG_SOURCE_APPLICATION:     std::cout << "Source: Application"; break;
	case GL_DEBUG_SOURCE_OTHER:           std::cout << "Source: Other"; break;
	} std::cout << std::endl;

	switch (type)
	{
	case GL_DEBUG_TYPE_ERROR:               std::cout << "Type: Error"; break;
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: std::cout << "Type: Deprecated Behaviour"; break;
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  std::cout << "Type: Undefined Behaviour"; break;
	case GL_DEBUG_TYPE_PORTABILITY:         std::cout << "Type: Portability"; break;
	case GL_DEBUG_TYPE_PERFORMANCE:         std::cout << "Type: Performance"; break;
	case GL_DEBUG_TYPE_MARKER:              std::cout << "Type: Marker"; break;
	case GL_DEBUG_TYPE_PUSH_GROUP:          std::cout << "Type: Push Group"; break;
	case GL_DEBUG_TYPE_POP_GROUP:           std::cout << "Type: Pop Group"; break;
	case GL_DEBUG_TYPE_OTHER:               std::cout << "Type: Other"; break;
	} std::cout << std::endl;

	switch (severity)
	{
	case GL_DEBUG_SEVERITY_HIGH:         std::cout << "Severity: high"; break;
	case GL_DEBUG_SEVERITY_MEDIUM:       std::cout << "Severity: medium"; break;
	case GL_DEBUG_SEVERITY_LOW:          std::cout << "Severity: low"; break;
	case GL_DEBUG_SEVERITY_NOTIFICATION: std::cout << "Severity: notification"; break;
	} std::cout << std::endl;
	std::cout << std::endl;
}

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


	//userInterface = new GraphicalUserInterface(window);

	
	activeScene.Init(CURRENT_WINDOW_WIDTH, CURRENT_WINDOW_HEIGHT);
	
	m_inputSystem = new InputSystem(window);
	glfwSetWindowUserPointer(window, m_inputSystem);
	
	
	m_cameraController = new CameraController(*m_inputSystem, *activeScene.camera);
	m_sceneInputController = new SceneInputController(*m_inputSystem, activeScene.environmentRotation);
	//inputHandler = new InputHandler();
	//inputHandler->Init(window, CURRENT_WINDOW_WIDTH, CURRENT_WINDOW_HEIGHT, &activeScene.camera, time, deltaTime, lastFrame);
	//glfwSetWindowUserPointer(window, inputHandler);



	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetScrollCallback(window, mouse_scroll_callback);
	glfwSetCursorPosCallback(window, mouse_cursor_callback);

}

App::~App()
{
	delete m_inputSystem;
	//delete userInterface;

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
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);

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


	int flags; glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
	if (flags & GL_CONTEXT_FLAG_DEBUG_BIT)
	{
		// initialize debug output 
	}

	if (flags & GL_CONTEXT_FLAG_DEBUG_BIT)
	{
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(glDebugOutput, nullptr);
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
	}
}



void App::run()
{
	//ImGuiIO& io = ImGui::GetIO();
	
	setupGlSettings();

	SceneRenderer sceneRenderer(SCR_WIDTH,SCR_HEIGHT);

	// ImGui Needs a refrence to the openGl Texture id so that we can later draw the renderTexture into the ImGui Image of the viewport Window
	//userInterface->viewportRenderTextureID = sceneRenderer.GetRenderTextureId();
	
	glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);

	while (!glfwWindowShouldClose(window))
	{

		float time = static_cast<float>(glfwGetTime());
		deltaSeconds = time - lastFrame;
		lastFrame = time;



		glfwPollEvents();
		//m_CameraController->UpdateTime(deltaTime); // idially forwarded by the input system
		m_inputSystem->Update(deltaSeconds);
		//m_inputSystem->UpdateKeyboardInputs();
		//userInterface->UpdateWindowPosition(window);
		

		//  ImGui Needs to be rendered before OpenGl Scene
		
		
		// proccess inputs
		bool processInput = true; //userInterface->IsMouseOverViewport();
		if (processInput)
		{
		//	inputHandler->proccessInputs = true;
		//	inputHandler->update(activeScene.environmentRotation);
		}
		else
		{
		//	inputHandler->proccessInputs = false;
		}


		//userInterface->Render();
		//std::cout << "imGui wantCaptureMouse: " << inputHandler->proccessInputs << std::endl;



		
		activeScene.Update();

		sceneRenderer.RenderScene(&activeScene, CURRENT_WINDOW_WIDTH, CURRENT_WINDOW_HEIGHT);


		glfwSwapBuffers(window);
	}
}


// GLFW CALLSBACKS
void framebuffer_size_callback(GLFWwindow* window, int width, int height) 
{
	glViewport(0, 0, width, height);
	InputSystem* inputSystem = (InputSystem*)glfwGetWindowUserPointer(window);
	inputSystem->UpdateWindowResizeInputs(width, height);
	inputSystem = nullptr;
	delete inputSystem;
}

void mouse_button_callback(GLFWwindow* window,int button, int action, int mods)
{
	InputSystem* inputSystem = (InputSystem*)glfwGetWindowUserPointer(window);
	inputSystem->UpdateMouseButtonInputs(button, action, mods);
	inputSystem = nullptr;
	delete inputSystem;
}
void mouse_cursor_callback(GLFWwindow* window, double posX, double posY)
{
	InputSystem* inputSystem = (InputSystem*)glfwGetWindowUserPointer(window);
	inputSystem->UpdateMouseCursorInputs(posX, posY);
	inputSystem = nullptr;
	delete inputSystem;
}
void mouse_scroll_callback(GLFWwindow* window, double offsetX, double offsetY)
{
	InputSystem* inputSystem = (InputSystem*)glfwGetWindowUserPointer(window);
	inputSystem->UpdateMouseScrollInputs(offsetX, offsetY);
	inputSystem = nullptr;
	delete inputSystem;
}
