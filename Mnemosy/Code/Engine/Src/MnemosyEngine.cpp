#include "Include/MnemosyConfig.h"
#include "Include/MnemosyEngine.h"

#include <GLFW/glfw3.h>

#include "Include/Core/Window.h"
#include "Include/Core/Logger.h"
#include "Include/Core/Log.h"
#include "Include/Core/Clock.h"
#include "Include/Core/FileDirectories.h"

#include "Include/Systems/Input/InputSystem.h"
#include "Include/Systems/SkyboxAssetRegistry.h"

#include "Include/Graphics/Material.h"
#include "Include/Graphics/Renderer.h"
#include "Include/Graphics/Skybox.h"
#include "Include/Graphics/RenderMesh.h"
#include "Include/Graphics/Camera.h"
#include "Include/Graphics/ImageBasedLightingRenderer.h"
#include "Include/Graphics/Light.h"
#include "Include/Graphics/Scene.h"

#include "Include/Gui/UserInterface.h"

namespace mnemosy
{
	// private singleton stuff
	MnemosyEngine::MnemosyEngine() {	}
	MnemosyEngine* MnemosyEngine::m_sInstance = nullptr;

	// public
	MnemosyEngine::~MnemosyEngine()
	{ }

	MnemosyEngine& MnemosyEngine::GetInstance()
	{
		if (!m_sInstance)
		{
			m_sInstance = new MnemosyEngine();
		}

		return *m_sInstance;
	}

	void MnemosyEngine::Initialize(const char* WindowTitle)
	{
		MNEMOSY_ASSERT(!m_isInitialized, "Engine::Initialize() has already been called");

		if (m_isInitialized)
			return;

		m_pLogger = std::make_unique<core::Logger>();


		MNEMOSY_INFO("Starting Mnemosy v{}.{}", MNEMOSY_VERSION_MAJOR, MNEMOSY_VERSION_MINOR);

		#ifdef MNEMOSY_CONFIG_DEBUG
			MNEMOSY_INFO("Configuration Debug");
		#else
			MNEMOSY_INFO("Configuration Release");
		#endif // MNEMOSY_CONFIG_DEBUG
		#ifdef MNEMOSY_PLATFORM_WINDOWS
			MNEMOSY_INFO("Platform Windows");
		#else
			MNEMOSY_INFO("Platform Linux");
		#endif // MNEMOSY_PLATFORM_WINDOWS



		MNEMOSY_TRACE("Initializing Subsystems");
		m_pWindow = new core::Window(WindowTitle);
		MNEMOSY_TRACE("Window Initialized");
		
		double timeBegin = glfwGetTime();


		// subsystems
		// order of initialization here matters
		
		m_clock = std::make_unique<core::Clock>();
		MNEMOSY_TRACE("Clock Initialized");
		
		m_pFileDirectories = std::make_unique<core::FileDirectories>(); // need to come before scene and image base lighting renderer
		MNEMOSY_TRACE("FileDirectories Initialized");



		m_pInputSystem = std::make_unique<systems::InputSystem>();
		MNEMOSY_TRACE("InputSystem Initialized");
		m_pSkyboxAssetRegistry = std::make_unique<systems::SkyboxAssetRegistry>();
		MNEMOSY_TRACE("SkyboxAssetsRegistry Initialized");



		m_pIbl_renderer = std::make_unique<graphics::ImageBasedLightingRenderer>();
		MNEMOSY_TRACE("ibl_Renderer Initialized");
		m_pRenderer = std::make_unique<graphics::Renderer>();
		MNEMOSY_TRACE("Renderer Initialized");


		m_pScene = std::make_unique<graphics::Scene>();
		MNEMOSY_TRACE("Scene Initialized");



		m_pUserInterface = std::make_unique<gui::UserInterface>();
		MNEMOSY_TRACE("UserInterface Initialized");

		
		m_pRenderer->SetPbrShaderBrdfLutUniforms();
		m_pRenderer->SetPbrShaderLightUniforms();
		m_pRenderer->SetShaderSkyboxUniforms();

		m_clock->capDeltaTime = false;

		m_isInitialized = true;


		MNEMOSY_INFO("End Startup");
		double timeEnd = glfwGetTime();
		MNEMOSY_INFO("TimeBeginStartup: {} ,TimeEndStartup: {}  StartupTime: {} ", timeBegin,timeEnd,(timeEnd-timeBegin));


		//fs::path meshsPath = GetFileDirectories().GetResourcesPath() / fs::path("Meshes");
		//MNEMOSY_DEBUG("MeshesPath: {}", meshsPath.generic_string());
		//GetFileDirectories().GetResourcesPath();
	}

	void MnemosyEngine::Run()
	{
		while (!glfwWindowShouldClose(&m_pWindow->GetWindow())) 
		{
			m_clock->Update();
			//MNEMOSY_TRACE("FPS: {} ", m_clock->GetFPS());

			
			glfwPollEvents();

			
			if (m_pUserInterface->WantCaptureInput())
			{
				m_pInputSystem->DontProcessUserInputs();
			}
			else
			{
				m_pInputSystem->ProcessUserInputs();
			}

			m_pInputSystem->Update(m_clock->GetDeltaSeconds());


			m_pScene->Update();


			// Rendering
			m_pRenderer->RenderScene(*m_pScene);

			m_pUserInterface->Render();


			glfwSwapBuffers(&m_pWindow->GetWindow());
		}

	}

	void MnemosyEngine::Shutdown()
	{

		m_pWindow->Shutdown();
	}

} // !mnemosy