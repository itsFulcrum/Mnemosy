#include "Include/MnemosyConfig.h"
#include "Include/MnemosyEngine.h"

#include <GLFW/glfw3.h>

#include "Include/Core/Window.h"
#include "Include/Core/Logger.h"
#include "Include/Core/Log.h"
#include "Include/Core/Clock.h"
#include "Include/Core/FileDirectories.h"
#include "Include/Core/Utils/DropHandler_Windows.h"

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

	void MnemosyEngine::Initialize(const char* WindowTitle) {
		//double timeBegin = glfwGetTime();

		MNEMOSY_ASSERT(!m_isInitialized, "Engine::Initialize() has already been called");

		if (m_isInitialized)
			return;

		m_pLogger = std::make_unique<core::Logger>();


		MNEMOSY_INFO("Starting Mnemosy v{}.{} - alpha", MNEMOSY_VERSION_MAJOR, MNEMOSY_VERSION_MINOR);

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

		m_pFileDirectories = std::make_unique<core::FileDirectories>(); // need to come before scene and image base lighting renderer
		MNEMOSY_TRACE("FileDirectories Initialized");
		

		m_pWindow = new core::Window(WindowTitle);
		MNEMOSY_TRACE("Window Initialized");
		
		// subsystems
		// Mnemosy::core
		m_pClock = std::make_unique<core::Clock>();
		m_pClock->capDeltaTime = true;
		MNEMOSY_TRACE("Clock Initialized");
		// order of initialization here matters
		
		
		m_pDropHandler = std::make_unique<core::DropHandler>();
		m_pDropHandler->Initialize(m_pWindow->GetWindow());
		MNEMOSY_TRACE("DropHandler Initialized");

		// mnemosy::systems
		m_pInputSystem = std::make_unique<systems::InputSystem>();
		MNEMOSY_TRACE("InputSystem Initialized");
		m_pSkyboxAssetRegistry = std::make_unique<systems::SkyboxAssetRegistry>();
		MNEMOSY_TRACE("SkyboxAssetsRegistry Initialized");
		m_pMaterialLibraryRegistry = std::make_unique<systems::MaterialLibraryRegistry>();
		MNEMOSY_TRACE("MaterialRegistry Initialized");
		m_pThumbnailManger = std::make_unique<systems::ThumbnailManager>();
		MNEMOSY_TRACE("ThumbnailManager Initialized");
		m_pTextureGenerationManager = std::make_unique<systems::TextureGenerationManager>();
		MNEMOSY_TRACE("TextureGenManager Initialized");
		m_pExportManager = std::make_unique<systems::ExportManager>();
		MNEMOSY_TRACE("ExportManager Initialized");



		// menmosy::graphcs
		double ibl_timeBegin = glfwGetTime();
		m_pIbl_renderer = std::make_unique<graphics::ImageBasedLightingRenderer>();
		double ibl_timeEnd = glfwGetTime();
		MNEMOSY_TRACE("ibl_Renderer Initialdized in {} Seconds", (ibl_timeEnd - ibl_timeBegin));
		

		double ren_timeBegin = glfwGetTime();
		m_pRenderer = std::make_unique<graphics::Renderer>();
		double ren_timeEnd = glfwGetTime();
		MNEMOSY_TRACE("Renderer Initialdized in {} Seconds", (ren_timeEnd - ren_timeBegin));
		//MNEMOSY_TRACE("Renderer Initialized");

		double timeSceneStart = glfwGetTime();
		m_pScene = std::make_unique<graphics::Scene>();
		double timeSceneEnd = glfwGetTime();
		m_pThumbnailScene = std::make_unique<graphics::ThumbnailScene>();

		MNEMOSY_TRACE("Scene Initialized {} seconds ",timeSceneEnd-timeSceneStart);

		m_pUserInterface = std::make_unique<gui::UserInterface>();
		MNEMOSY_TRACE("UserInterface Initialized");

		
		m_pRenderer->SetPbrShaderBrdfLutUniforms();
		m_pRenderer->SetPbrShaderLightUniforms(m_pScene->GetLight());
		m_pRenderer->SetShaderSkyboxUniforms(m_pScene->GetSkybox());


		m_isInitialized = true;


		//double timeEnd = glfwGetTime();
		//MNEMOSY_INFO("Mnemosy Engine Initialized {} Seconds", (timeEnd - timeBegin)); 

	}

	void MnemosyEngine::Run() {

		while (!glfwWindowShouldClose(&m_pWindow->GetWindow()))  {

			m_pClock->Update();
						
			glfwPollEvents();
						
			if (m_pUserInterface->WantCaptureInput()) {
				m_pInputSystem->DontProcessUserInputs();
			}
			else {
				m_pInputSystem->ProcessUserInputs();
			}

			m_pInputSystem->Update(m_pClock->GetDeltaSeconds());

			m_pThumbnailManger->Update();
			m_pScene->Update();

			// Rendering
			m_pRenderer->RenderScene(*m_pScene);
			m_pUserInterface->Render();

			glfwSwapBuffers(&m_pWindow->GetWindow());
		
		} // End of main loop

	}

	void MnemosyEngine::Shutdown() {	

		m_pWindow->Shutdown();
		delete m_pWindow;
		m_pWindow = nullptr;

		delete m_sInstance;
	}

} // !mnemosy