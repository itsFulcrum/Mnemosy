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


	// hides mouse cursor and keeps it at center of the window
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);


	mainCamera = new Camera();
	mainCamera->init(CURRENT_WINDOW_WIDTH, CURRENT_WINDOW_HEIGHT);

	
	inputHandler = new InputHandler();
	inputHandler->Init(window,CURRENT_WINDOW_WIDTH, CURRENT_WINDOW_HEIGHT, mainCamera,time,deltaTime,lastFrame);
	
	glfwSetWindowUserPointer(window, inputHandler);

	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetScrollCallback(window, mouse_scroll_callback);
	glfwSetCursorPosCallback(window, mouse_cursor_callback);

	
}

App::~App()
{
	delete mainCamera;
	delete inputHandler;
	glfwTerminate();
}


void App::setupGLFWWindow(const char* WindowTileName)
{
	// initialize glfw with opengl versions;
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_SAMPLES, 4);


	// create GLFW window
	window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT,WindowTileName, NULL, NULL);


	if (window == NULL)
	{
		std::cout << "Warning!: Failed to create GLFW window" << std::endl;
		glfwTerminate();
		//return-1;
	}
	glfwMakeContextCurrent(window);
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

// this will eventually be in a seperate class
void App::setupScene()
{
	mainCamera->cameraMode = CAMERA_MODE_FLY;
	mainCamera->position = glm::vec3(0.0f, 0.0f, 3.0f);

	
}

void App::run()
{
	
	setupGlSettings();
	setupScene();

	SceneRenderer sceneRenderer;

	ModelLoader modelLoader;



	Shader skyboxShader("src/shaders/skybox.vert", "src/shaders/skybox.frag");
	//Shader lightShader("src/shaders/textureVertex.vert", "src/shaders/unlit.frag");
	Shader pbrShader("src/shaders/pbrVertex.vert", "src/shaders/pbrFragment.frag");

	// SKYBOX
	Object skybox;
	skybox.modelData = modelLoader.LoadModelDataFromFile("fbx/skyboxMesh.fbx");

	Cubemap cubemapTexture("textures/spruit_sunrise.hdr", 1024, true);


	// MESH OBJECT
	Object Suzanne;
	Suzanne.modelData = modelLoader.LoadModelDataFromFile("fbx/SuzanneSmooth.fbx");
	Suzanne.rotation.x = -90.0f;

	PbrMaterial material;
	material.assignShader(pbrShader);
	material.assignTexture(ALBEDO, "textures/panel_albedo.png");
	material.assignTexture(ROUGHNESS, "textures/panel_roughness.png");
	material.assignTexture(METALLIC, "textures/panel_metallic.png");
	material.assignTexture(NORMAL, "textures/panel_normal.png");
	material.assignTexture(AMBIENTOCCLUSION, "textures/panel_ao.png");
	material.assignTexture(EMISSION, "textures/panel_emissive.png");
	
	material.EmissionStrength = 0.0;
	material.Albedo = glm::vec3(0.1f, 0.8f, 0.0f);
	material.Emission = glm::vec3(0.0f, 0.3f, 0.7f);


	// LIGHT OBJECT (as mesh)
	Object Light;
	Light.modelData = modelLoader.LoadModelDataFromFile("fbx/icoSphere.fbx");
	Light.position = glm::vec3(6.0f, 4.0f, 0.0f);
	Light.scale = glm::vec3(0.7f, 0.7f, 0.7f);


	PbrMaterial lightMaterial;
	lightMaterial.assignShader(pbrShader);
	lightMaterial.Albedo = glm::vec3(1.0f, 1.0f, 1.0f);
	lightMaterial.Roughness = 1.0;
	lightMaterial.Emission = glm::vec3(1.0f, 1.0f, 1.0f);
	lightMaterial.EmissionStrength = 2.0f;


	glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);

	while (!glfwWindowShouldClose(window))
	{
		
		float time = static_cast<float>(glfwGetTime());
		deltaTime = time - lastFrame;
		lastFrame = time;

		inputHandler->update();
		
		sceneRenderer.ClearFrame(0.02f, 0.08f, 0.14f);



		glm::vec3 lightPosition = glm::vec3(Light.position.x, Light.position.y, Light.position.z);
		glm::vec3 cameraPosition = glm::vec3(mainCamera->position.x, mainCamera->position.y, mainCamera->position.z);
		
		sceneRenderer.SetProjectionMatrix(mainCamera->GetProjectionMatrix());
		sceneRenderer.SetViewMatrix(mainCamera->GetViewMatrix());
		// should probably treat camera position seperatly as the other variables dont need to be set each frame and can be updated on specific events
		sceneRenderer.SetPbrShaderGlobalSceneUniforms(pbrShader, cubemapTexture, lightPosition, cameraPosition);

		// draw calls
		sceneRenderer.RenderMesh(Suzanne,material);
		// render light source ( as an object placeholder atm
		sceneRenderer.RenderMesh(Light,lightMaterial);
		// Render skybox last
		sceneRenderer.RenderSkybox(skybox, skyboxShader,cubemapTexture);

	
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
}


// CALLSBACKS
void framebuffer_size_callback(GLFWwindow* window, int width, int height) 
{
	glViewport(0, 0, width, height);
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
