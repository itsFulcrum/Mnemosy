

#include "App.h"


App::App()
{

	setupGLFWWindow("Mnemosy Texture library");
	setupImGui();
	

	// initialize glad "We pass GLAD the function to load the address of the OpenGL function pointers which is OS-specific"
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Warning!: Failed to initialize GLAD" << std::endl;
		glfwTerminate();
	}


	// hides mouse cursor and keeps it at center of the window
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);


	mainCamera = new Camera();
	mainCamera->Init(CURRENT_WINDOW_WIDTH, CURRENT_WINDOW_HEIGHT);

	
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


	// ImGui Cleanup
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

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

void App::setupImGui()
{	
	// GL 4.5 + GLSL 130
	const char* glsl_version = "#version 450";

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	io = ImGui::GetIO(); (void)io;
	//ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows
	//io.ConfigViewportsNoAutoMerge = true;
	//io.ConfigViewportsNoTaskBarIcon = true;

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsLight();

	// When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
	ImGuiStyle& style = ImGui::GetStyle();
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		style.WindowRounding = 0.0f;
		style.Colors[ImGuiCol_WindowBg].w = 1.0f;
	}

	// Setup Platform/Renderer backends
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init(glsl_version);

	// Load Fonts
	// - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
	// - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
	// - If the file cannot be loaded, the function will return a nullptr. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
	// - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
	// - Use '#define IMGUI_ENABLE_FREETYPE' in your imconfig file to use Freetype for higher quality font rendering.
	// - Read 'docs/FONTS.md' for more instructions and details.
	// - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
	// - Our Emscripten build process allows embedding fonts to be accessible at runtime from the "fonts/" folder. See Makefile.emscripten for details.
	//io.Fonts->AddFontDefault();
	//io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\segoeui.ttf", 18.0f);
	//io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
	//io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
	//io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
	//ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, nullptr, io.Fonts->GetGlyphRangesJapanese());
	//IM_ASSERT(font != nullptr);

	ImFont* font = io.Fonts->AddFontFromFileTTF("resources/fonts/RobotoRegular.ttf", 20.0f);

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

	ModelLoader modelLoader;
	SceneRenderer sceneRenderer;




	Shader skyboxShader("src/shaders/skybox.vert", "src/shaders/skybox.frag");
	//Shader lightShader("src/shaders/textureVertex.vert", "src/shaders/unlit.frag");
	Shader pbrShader("src/shaders/pbrVertex.vert", "src/shaders/pbrFragment.frag");

	// SKYBOX
	Object skybox;
	skybox.modelData = modelLoader.LoadModelDataFromFile("fbx/skyboxMesh.fbx");

	Cubemap cubemapTexture("textures/kloppenheim.hdr", 1024, true);


	// MESH OBJECT
	Object Suzanne;
	Suzanne.modelData = modelLoader.LoadModelDataFromFile("fbx/TestObj.fbx");
	Suzanne.position = glm::vec3(0.0f, 0.0f, 0.0f);
	Suzanne.rotation.x = -90.0f;

	PbrMaterial material;
	material.assignShader(pbrShader);
	material.assignTexture(ALBEDO, "textures/panel_albedo.png");
	material.assignTexture(ROUGHNESS, "textures/panel_roughness.png");
	material.assignTexture(METALLIC, "textures/panel_metallic.png");
	material.assignTexture(NORMAL, "textures/panel_normal.png");
	material.assignTexture(AMBIENTOCCLUSION, "textures/panel_ao.png");
	material.assignTexture(EMISSION, "textures/panel_emissive.png");
	
	material.EmissionStrength = 1.0;
	material.Albedo = glm::vec3(0.1f, 0.8f, 0.0f);
	material.Emission = glm::vec3(0.0f, 0.3f, 0.7f);
	material.Metallic = 1.0f;
	material.Roughness = 0.1f;

	// LIGHT OBJECT (as mesh)
	Object Light;
	Light.modelData = modelLoader.LoadModelDataFromFile("fbx/ArrowZ.fbx");
	Light.position = glm::vec3(0.0f, 1.0f, 0.0f);
	Light.rotation = glm::vec3(45.0f, 0.0f, 0.0f);
	Light.scale = glm::vec3(0.1f, 0.1f, 0.1f);

	std::cout << "light forward: x:" << Light.GetForward().x << " y: " << Light.GetForward().y << " z: " << Light.GetForward().z << std::endl;

	PbrMaterial lightMaterial;
	lightMaterial.assignShader(pbrShader);
	lightMaterial.Albedo = glm::vec3(1.0f, 1.0f, 1.0f);
	lightMaterial.Roughness = 1.0;
	lightMaterial.Emission = glm::vec3(1.0f, 1.0f, 1.0f);
	lightMaterial.EmissionStrength = 10.0f;



	glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
	float environmentRotation = 0.0f;


	while (!glfwWindowShouldClose(window))
	{
		
		float time = static_cast<float>(glfwGetTime());
		deltaTime = time - lastFrame;
		lastFrame = time;

		float sinTime = sin(time)+1;


		inputHandler->update(environmentRotation);
		


		sceneRenderer.ClearFrame(0.02f, 0.08f, 0.14f);

		Light.rotation.y = glm::degrees(environmentRotation);


		glm::vec3 cameraPosition = glm::vec3(mainCamera->position.x, mainCamera->position.y, mainCamera->position.z);
		
		sceneRenderer.SetProjectionMatrix(mainCamera->GetProjectionMatrix());
		sceneRenderer.SetViewMatrix(mainCamera->GetViewMatrix());
		// should probably treat camera position seperatly as the other variables dont need to be set each frame and can be updated on specific events
		sceneRenderer.SetPbrShaderGlobalSceneUniforms(pbrShader, cubemapTexture, -Light.GetForward(), lightMaterial.EmissionStrength, cameraPosition, environmentRotation);

		// draw calls
		sceneRenderer.RenderMesh(Suzanne,material);
		// render light source ( as an object placeholder atm
		sceneRenderer.RenderMesh(Light,lightMaterial);
		// Render skybox last

		glm::vec3 skyboxColorTint = glm::vec3(1, 1, 1);
		sceneRenderer.RenderSkybox(skybox, skyboxShader,cubemapTexture,environmentRotation,skyboxColorTint);


		// IM GUI

			// Our state
		bool show_demo_window = true;
		bool show_another_window = false;
		ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);


		// Start the Dear ImGui frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		// 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
		if (show_demo_window)
			ImGui::ShowDemoWindow(&show_demo_window);

		// 2. Show a simple window that we create ourselves. We use a Begin/End pair to create a named window.
		{
			static float f = 0.0f;
			static int counter = 0;

			ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

			ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
			ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
			ImGui::Checkbox("Another Window", &show_another_window);

			ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
			ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

			if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
				counter++;
			ImGui::SameLine();
			ImGui::Text("counter = %d", counter);

			ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
			ImGui::End();
		}

		// 3. Show another simple window.
		if (show_another_window)
		{
			ImGui::Begin("Another Window", &show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
			ImGui::Text("Hello from another window!");
			if (ImGui::Button("Close Me"))
				show_another_window = false;
			ImGui::End();
		}

		// Rendering
		ImGui::Render();
		//int display_w, display_h;
		//glfwGetFramebufferSize(window, &display_w, &display_h);
		//glViewport(0, 0, display_w, display_h);
		//glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
		//glClear(GL_COLOR_BUFFER_BIT);
		
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		// Update and Render additional Platform Windows
		// (Platform functions may change the current OpenGL context, so we save/restore it to make it easier to paste this code elsewhere.
		//  For this specific demo app we could also call glfwMakeContextCurrent(window) directly)
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			GLFWwindow* backup_current_context = glfwGetCurrentContext();
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
			glfwMakeContextCurrent(backup_current_context);
		}


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


//glm::vec3 Rotate_About_Axis_Radians_float(glm::vec3 In, glm::vec3 Axis, float Rotation)
//{
//	float s = sin(Rotation);
//	float c = cos(Rotation);
//	float one_minus_c = 1.0 - c;
//
//	Axis = normalize(Axis);
//
//	glm::mat4x4 rot_mat = glm::mat3x3(1.0f);
//	//glm::rotate(rot_mat, );
//	rot_mat = glm::rotate(rot_mat, glm::radians(Rotation), glm::vec3(0.0f, 1.0f, 0.0f));
//	/*
//	one_minus_c * Axis.x * Axis.x + c,            one_minus_c * Axis.x * Axis.y - Axis.z * s,     one_minus_c * Axis.z * Axis.x + Axis.y * s,
//	one_minus_c * Axis.x * Axis.y + Axis.z * s,   one_minus_c * Axis.y * Axis.y + c,              one_minus_c * Axis.y * Axis.z - Axis.x * s,
//	one_minus_c * Axis.z * Axis.x - Axis.y * s,   one_minus_c * Axis.y * Axis.z + Axis.x * s,     one_minus_c * Axis.z * Axis.z + c
//	*/
//
//	rot_mat[0][0] = one_minus_c * Axis.x * Axis.x + c;
//	rot_mat[1][0] = one_minus_c * Axis.x * Axis.y - Axis.z * s;
//	rot_mat[2][0] = one_minus_c * Axis.z * Axis.x + Axis.y * s;
//
//	rot_mat[0][1] = one_minus_c * Axis.x * Axis.y + Axis.z * s;
//	rot_mat[1][1] = one_minus_c * Axis.y * Axis.y + c;
//	rot_mat[2][1] = one_minus_c * Axis.y * Axis.z - Axis.x * s;
//
//	rot_mat[0][2] = one_minus_c * Axis.z * Axis.x - Axis.y * s;
//	rot_mat[1][2] = one_minus_c * Axis.y * Axis.z + Axis.x * s;
//	rot_mat[2][2] = one_minus_c * Axis.z * Axis.z + c;
//
//	//return rot_mat * In;
//}