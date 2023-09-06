#include "App.h"


App::App()
{

	// initialize glfw with opengl versions;
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);


	// create GLFW window
	 window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Baytalmawadi", NULL, NULL);
	

	if (window == NULL)
	{
		std::cout << "Warning!: Failed to create GLFW window" << std::endl;
		glfwTerminate();
		//return-1;
	}
	glfwMakeContextCurrent(window);

	// initialize glad "We pass GLAD the function to load the address of the OpenGL function pointers which is OS-specific"
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Warning!: Failed to initialize GLAD" << std::endl;
		glfwTerminate();
	}



	// register GLFW window resize callback to adjust openGl viewport on window resizes

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

	
	// set openGl rendering viewport at position and width/height
	glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);

	
}

App::~App()
{
	delete mainCamera;
	delete inputHandler;
	glfwTerminate();
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
	

	
	//enable depth testing
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
	glDisable(GL_FRAMEBUFFER_SRGB);

	// draw Polygons opaque (Fully shaded)
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

	// cubemap


	Cubemap cubemapTexture("textures/market.hdr", 1024, true);


	Shader skyboxShader("src/shaders/skybox.vert", "src/shaders/skybox.frag");
	Shader lightShader("src/shaders/textureVertex.vert", "src/shaders/unlit.frag");
	Shader pbrShader("src/shaders/pbrVertex.vert", "src/shaders/pbrFragment.frag");


	// material
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


	material.removeTexture(EMISSION);
	//material.removeTexture(ROUGHNESS);

	//Object Suzanne("fbx/unitPlane.fbx");
	Object Suzanne("fbx/SuzanneSmooth.fbx");
	// icoshere model
	Object Light("fbx/icoSphere.fbx");
	Light.position = glm::vec3(6.0f, 4.0f, 0.0f);
	Light.scale = glm::vec3(0.7f, 0.7f, 0.7f);

	Object skybox("fbx/skyboxMesh.fbx");

	glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);

	while (!glfwWindowShouldClose(window))
	{
		
		float time = static_cast<float>(glfwGetTime());
		deltaTime = time - lastFrame;
		lastFrame = time;

		inputHandler->update();

		// rendering commands
		glClearColor(0.02f, 0.08f, 0.14f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		
		glm::mat4 viewMatrix = mainCamera->GetViewMatrix();
		glm::mat4 projectionMatrix = mainCamera->GetProjectionMatrix();


		//Suzanne.rotation.x = (0.5 * sin(time * 0.1) +0.5)*360;
		Suzanne.rotation.x = -90.0f;
		//Suzanne.rotation.y = time * 5.0f;
		Suzanne.rotation.z = 0.0f;

		pbrShader.use();
		pbrShader.setUniformFloat3("_lightPositionWS", Light.position.x, Light.position.y, Light.position.z);
		pbrShader.setUniformFloat("_lightStrength", 1);
		pbrShader.setUniformFloat3("_cameraPositionWS", mainCamera->position.x, mainCamera->position.y, mainCamera->position.z);

		cubemapTexture.BindIrradianceCubemap(7);
		pbrShader.setUniformInt("_irradianceMap", 7);
		cubemapTexture.BindPrefilteredCubemap(8);
		pbrShader.setUniformInt("_prefilterMap", 8);
		cubemapTexture.BindBrdfLutTexture(9);
		pbrShader.setUniformInt("_brdfLUT", 9);


		material.setMaterialUniforms();

		Suzanne.Render(pbrShader, viewMatrix, projectionMatrix);




		// render light source ( as an object placeholder atm
		lightShader.use();
		Light.Render(lightShader, viewMatrix, projectionMatrix);

		// Render skybox last
		glDepthFunc(GL_LEQUAL);
		skyboxShader.use();
		cubemapTexture.BindColorCubemap(0);
		skyboxShader.setUniformInt("_skybox", 0);
		glm::mat4 skyboxViewMatrix = glm::mat4(glm::mat3(viewMatrix));
		skybox.Render(skyboxShader, skyboxViewMatrix, projectionMatrix);
		glDepthFunc(GL_LESS);


		// swap buffers and display new frame
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
