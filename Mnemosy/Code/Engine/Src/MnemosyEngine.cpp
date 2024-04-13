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
#include "Include/Systems/MaterialLibraryRegistry.h"
#include "Include/Systems/ThumbnailManager.h"
#include "Include/Systems/TextureGenerationManager.h"
#include "Include/Systems/ExportManager.h"

#include "Include/Graphics/Material.h"
#include "Include/Graphics/Renderer.h"
#include "Include/Graphics/Skybox.h"
#include "Include/Graphics/RenderMesh.h"
#include "Include/Graphics/Camera.h"
#include "Include/Graphics/ImageBasedLightingRenderer.h"
#include "Include/Graphics/Light.h"
#include "Include/Graphics/Scene.h"
#include "Include/Graphics/ThumbnailScene.h"

#include "Include/Gui/UserInterface.h"

#include "Include/Core/Utils/PlatfromUtils_Windows.h"
#include "Include/Core/Utils/DropManager_Windows.h"

namespace mnemosy
{
	// private singleton stuff
	MnemosyEngine::MnemosyEngine() {	}
	MnemosyEngine* MnemosyEngine::m_sInstance = nullptr;

	// public
	MnemosyEngine::~MnemosyEngine()
	{ 
		
	}

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
		double timeBegin = glfwGetTime();

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


		m_pFileDirectories = std::make_unique<core::FileDirectories>(); // need to come before scene and image base lighting renderer
		//MNEMOSY_TRACE("FileDirectories Initialized");
		

		MNEMOSY_TRACE("Initializing Subsystems");
		m_pWindow = new core::Window(WindowTitle);
		//MNEMOSY_TRACE("Window Initialized");
		


		// subsystems
		// Mnemosy::core
		m_clock = std::make_unique<core::Clock>();
		//MNEMOSY_TRACE("Clock Initialized");
		// order of initialization here matters
		
		// mnemosy::systems
		m_pInputSystem = std::make_unique<systems::InputSystem>();
		//MNEMOSY_TRACE("InputSystem Initialized");
		m_pSkyboxAssetRegistry = std::make_unique<systems::SkyboxAssetRegistry>();
		//MNEMOSY_TRACE("SkyboxAssetsRegistry Initialized");
		m_pMaterialLibraryRegistry = std::make_unique<systems::MaterialLibraryRegistry>();
		m_pThumbnailManger = std::make_unique<systems::ThumbnailManager>();
		m_pTextureGenerationManager = std::make_unique<systems::TextureGenerationManager>();
		m_pExportManager = std::make_unique<systems::ExportManager>();

		// menmosy::graphcs
		m_pIbl_renderer = std::make_unique<graphics::ImageBasedLightingRenderer>();
		//MNEMOSY_TRACE("ibl_Renderer Initialized");
		m_pRenderer = std::make_unique<graphics::Renderer>();
		//MNEMOSY_TRACE("Renderer Initialized");

		double timeSceneStart = glfwGetTime();
		m_pScene = std::make_unique<graphics::Scene>();
		double timeSceneEnd = glfwGetTime();
		m_pThumbnailScene = std::make_unique<graphics::ThumbnailScene>();

		MNEMOSY_TRACE("Scene Initialized {} seconds ",timeSceneEnd-timeSceneStart);



		m_pUserInterface = std::make_unique<gui::UserInterface>();
		//MNEMOSY_TRACE("UserInterface Initialized");

		
		m_pRenderer->SetPbrShaderBrdfLutUniforms();
		m_pRenderer->SetPbrShaderLightUniforms(m_pScene->GetLight());
		m_pRenderer->SetShaderSkyboxUniforms(m_pScene->GetSkybox());

		m_clock->capDeltaTime = true;

		m_isInitialized = true;


		double timeEnd = glfwGetTime();
		MNEMOSY_INFO("Mnemosy Engine Initialized {} Seconds", (timeEnd - timeBegin));

	}

	void MnemosyEngine::Run()
	{

		core::DropManager* dm = new core::DropManager();
		core::FileDialogs::RegisterDropManager(dm);
		//core::FileDialogs::RegisterDropManager(dm);

		
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
			m_pThumbnailManger->Update();

			//m_pThumbnailScene->Update();

			// Rendering
			m_pRenderer->RenderScene(*m_pScene);
			//m_pRenderer->RenderThumbnailScene();

			m_pUserInterface->Render();


			glfwSwapBuffers(&m_pWindow->GetWindow());
		}

	}

	void MnemosyEngine::Shutdown()
	{

		m_pWindow->Shutdown();
		delete m_pWindow;
		m_pWindow = nullptr;

		delete m_sInstance;
	}

} // !mnemosy