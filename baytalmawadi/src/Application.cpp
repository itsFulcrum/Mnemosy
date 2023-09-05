
#include <config.h>



void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double offsetX, double offsetY);
//Settings
const unsigned int SCR_WIDTH = 1024;
const unsigned int SCR_HEIGHT = 800;
unsigned int CURRENT_WINDOW_WIDTH = SCR_WIDTH;
unsigned int CURRENT_WINDOW_HEIGHT = SCR_HEIGHT;

float deltaTime = 0.0;
float lastFrame = 0.0;


// mouse
float mouseLastX = SCR_WIDTH * 0.5;
float mouseLastY = SCR_HEIGHT * 0.5;
bool firstMouseInput = true;


Camera mainCamera(CURRENT_WINDOW_WIDTH, CURRENT_WINDOW_HEIGHT);


int main() {
	// initialize glfw with opengl versions;
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);


	// create GLFW window
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "learnGL", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Warning!: Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return-1;
	}
	glfwMakeContextCurrent(window);
	// register GLFW window resize callback to adjust openGl viewport on window resizes

	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);

	glfwSetScrollCallback(window, scroll_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);

	// initialize glad "We pass GLAD the function to load the address of the OpenGL function pointers which is OS-specific"
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Warning!: Failed to initialize GLAD" << std::endl;
		return -1;
	}

	// set openGl rendering viewport at position and width/height
	glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);


	mainCamera.cameraMode = CAMERA_MODE_FLY;
	mainCamera.position = glm::vec3(0.0f, 0.0f, 3.0f);

	// generate a texture
	Texture albedoTexture;
	Texture normalTexture;
	Texture roughnessTexture;
	Texture metallicTexture;
	Texture emissiveTexture;
	Texture aoTexture;

	albedoTexture.generateFromFile("src/textures/panel_albedo.png", true, true);
	normalTexture.generateFromFile("src/textures/panel_normal.png", true, true);
	roughnessTexture.generateFromFile("src/textures/panel_roughness.png", true, true);
	metallicTexture.generateFromFile("src/textures/panel_metallic.png", true, true);
	emissiveTexture.generateFromFile("src/textures/panel_emissive.png", true, true);
	aoTexture.generateFromFile("src/textures/panel_ao.png", true, true);


	// cubemap


	Cubemap cubemapTexture("src/textures/kloppenheim.hdr", 1024, true);


	Shader skyboxShader("src/shaders/skybox.vert", "src/shaders/skybox.frag");
	Shader lightShader("src/shaders/textureVertex.vert", "src/shaders/unlit.frag");
	Shader pbrShader("src/shaders/pbrVertex.vert", "src/shaders/pbrFragment.frag");


	//Object Suzanne("fbx/unitPlane.fbx");
	Object Suzanne("src/fbx/SuzanneSmooth.fbx");
	// icoshere model
	Object Light("src/fbx/icoSphere.fbx");
	Light.position = glm::vec3(6.0f, 4.0f, 0.0f);
	Light.scale = glm::vec3(0.7f, 0.7f, 0.7f);

	Object skybox("src/fbx/skyboxMesh.fbx");


	// wireframe mode
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	// number of attributes we can set in the shader supported by hardware
	int nrAttributes;
	glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &nrAttributes);
	std::cout << "Maximum nr of vertex attributes supported: " << nrAttributes << std::endl;

	//enable depth testing
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

	glDisable(GL_FRAMEBUFFER_SRGB);
	// hides mouse cursor and keeps it at center of the window
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
	// RenderLoop
	while (!glfwWindowShouldClose(window))
	{
		// check input
		processInput(window);

		float time = static_cast<float>(glfwGetTime());
		deltaTime = time - lastFrame;
		lastFrame = time;


		// rendering commands
		glClearColor(0.02f, 0.08f, 0.14f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// draw Polygons opaque
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);


		glm::mat4 viewMatrix = mainCamera.GetViewMatrix();
		glm::mat4 projectionMatrix = mainCamera.GetProjectionMatrix();


		//Suzanne.rotation.x = (0.5 * sin(time * 0.1) +0.5)*360;
		Suzanne.rotation.x = -90.0f;
		//Suzanne.rotation.y = time * 5.0f;
		Suzanne.rotation.z = 0.0f;

		pbrShader.use();
		pbrShader.setUniformFloat3("_lightPositionWS", Light.position.x, Light.position.y, Light.position.z);
		pbrShader.setUniformFloat("_lightStrength", 1);
		pbrShader.setUniformFloat3("_cameraPositionWS", mainCamera.position.x, mainCamera.position.y, mainCamera.position.z);

		cubemapTexture.BindIrradianceCubemap(7);
		pbrShader.setUniformInt("_irradianceMap", 7);
		cubemapTexture.BindPrefilteredCubemap(8);
		pbrShader.setUniformInt("_prefilterMap", 8);
		cubemapTexture.BindBrdfLutTexture(9);
		pbrShader.setUniformInt("_brdfLUT", 9);



		albedoTexture.BindToLocation(0);
		pbrShader.setUniformInt("_albedoMap", 0);
		normalTexture.BindToLocation(1);
		pbrShader.setUniformInt("_normalMap", 1);
		roughnessTexture.BindToLocation(2);
		pbrShader.setUniformInt("_roughnessMap", 2);
		metallicTexture.BindToLocation(3);
		pbrShader.setUniformInt("_metallicMap", 3);
		emissiveTexture.BindToLocation(6);
		pbrShader.setUniformInt("_emissionMap", 6);
		// ao location 4
		// height map location 5
		// emmission location 6

		Suzanne.Render(pbrShader, viewMatrix, projectionMatrix);




		// render light source ( as an object placeholder atm
		//lightShader.use();
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

	// delete all GLFW's recourses
	glfwTerminate();
	return 0;
}


void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// match new window size upon resizing to openGl render area
	CURRENT_WINDOW_WIDTH = width;
	CURRENT_WINDOW_HEIGHT = height;
	glViewport(0, 0, width, height);
	// main camera need to know screen size to calculate the correct projection matrix
	mainCamera.updateScreenSize((unsigned int)width, (unsigned int)height);
}

void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);




	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
	{
		mainCamera.ProcessKeyboardInput(CAMERA_FORWARD, deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
	{
		mainCamera.ProcessKeyboardInput(CAMERA_LEFT, deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
	{
		mainCamera.ProcessKeyboardInput(CAMERA_BACKWARD, deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
	{
		mainCamera.ProcessKeyboardInput(CAMERA_RIGHT, deltaTime);
	}

}

void mouse_callback(GLFWwindow* window, double posX, double posY)
{
	if (firstMouseInput)
	{
		mouseLastX = (float)posX;
		mouseLastY = (float)posY;
		firstMouseInput = false;
	}

	// called each time the mouse is moved
	float mouseOffsetX = (float)posX - mouseLastX;
	float mouseOffsetY = mouseLastY - (float)posY;
	mouseLastX = (float)posX;
	mouseLastY = (float)posY;


	mainCamera.ProessMouseInput(mouseOffsetX, mouseOffsetY);
}
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
	{
		mainCamera.cameraMode = CAMERA_MODE_EDIT;
	}
	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE)
	{
		mainCamera.cameraMode = CAMERA_MODE_FLY;
	}
}
void scroll_callback(GLFWwindow* window, double offsetX, double offsetY)
{
	mainCamera.ProcessMouseScrollInput((float)offsetY, deltaTime);
}