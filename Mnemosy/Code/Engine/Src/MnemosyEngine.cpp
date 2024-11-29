#include "Include/MnemosyConfig.h"
#include "Include/MnemosyEngine.h"


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
#include "Include/Systems/MeshRegistry.h"

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

#include <GLFW/glfw3.h>

namespace mnemosy
{
	// private singleton stuff
	MnemosyEngine::MnemosyEngine() {	}
	MnemosyEngine* MnemosyEngine::m_sInstance = nullptr;

	// public
	MnemosyEngine::~MnemosyEngine()
	{ }

	MnemosyEngine& MnemosyEngine::GetInstance() {
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

		MNEMOSY_INFO("Starting Mnemosy v{}.{}-{}", MNEMOSY_VERSION_MAJOR, MNEMOSY_VERSION_MINOR,MNEMOSY_VERSION_SUFFIX);


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

		
		m_arena_persistent.arena_init_allocate_buffer(2048); //1024000 = 1 mb memory block


		//MNEMOSY_TRACE("Initializing Subsystems");
		//  === !!!  order of initialization here matters !!!! ===

		m_pFileDirectories = arena_placement_new(core::FileDirectories);// need to come before scene and image base lighting renderer
		m_pFileDirectories->Init();

		m_pWindow = arena_placement_new(core::Window);
		m_pWindow->Init(WindowTitle);

		#ifdef MNEMOSY_CONFIG_DISABLE_VSYNC
		m_pWindow->EnableVsync(false);
		#endif

		MNEMOSY_DEBUG("Window created");
		
		// subsystems
		// Mnemosy::core
		m_pClock = arena_placement_new(core::Clock);
		m_pClock->Init();

		
		m_pDropHandler = arena_placement_new(core::DropHandler);
		m_pDropHandler->Init(m_pWindow->GetWindow());

		// mnemosy::systems
		m_pInputSystem = arena_placement_new(systems::InputSystem);
		m_pInputSystem->Init();

		m_pSkyboxAssetRegistry = arena_placement_new(systems::SkyboxAssetRegistry);
		m_pSkyboxAssetRegistry->Init();

		m_pMaterialLibraryRegistry = arena_placement_new(systems::MaterialLibraryRegistry);
		m_pMaterialLibraryRegistry->Init();
		
		m_pThumbnailManger = arena_placement_new(systems::ThumbnailManager);
		m_pThumbnailManger->Init();

		m_pTextureGenerationManager = arena_placement_new(systems::TextureGenerationManager);
		m_pTextureGenerationManager->Init();


		m_pExportManager = arena_placement_new(systems::ExportManager);
		m_pExportManager->Init(),

			m_pMeshRegistry = arena_placement_new(systems::MeshRegistry);
		m_pMeshRegistry->Init();


		// menmosy::graphcs
		MNEMOSY_DEBUG("Initializing Renderer");
		m_pIbl_renderer =  arena_placement_new(graphics::ImageBasedLightingRenderer);
		m_pIbl_renderer->Init();

		m_pRenderer = arena_placement_new(graphics::Renderer);
		m_pRenderer->Init();


		MNEMOSY_DEBUG("Loading Scenes");
		m_pScene = arena_placement_new(graphics::Scene);
		m_pScene->Init();

		m_pThumbnailScene = arena_placement_new(graphics::ThumbnailScene);
		m_pThumbnailScene->Init();

		m_pUserInterface =  arena_placement_new(gui::UserInterface);
		m_pUserInterface->Init();

		m_pRenderer->SetPbrShaderBrdfLutUniforms();
		m_pRenderer->SetPbrShaderLightUniforms(m_pScene->GetLight());
		m_pRenderer->SetShaderSkyboxUniforms(m_pScene->GetSkybox());


		m_isInitialized = true;
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
			//m_pScene->Update();

			// Rendering
			m_pRenderer->HotReloadPbrShader(m_pClock->GetDeltaSeconds());
			m_pRenderer->RenderScene(*m_pScene, m_pMaterialLibraryRegistry->GetEntryTypeToRenderWith());
			m_pUserInterface->Render();

			glfwSwapBuffers(&m_pWindow->GetWindow());
			
		} // End of main loop

	}

	void MnemosyEngine::Shutdown() {	
		
		m_pMaterialLibraryRegistry->ActiveLibEntry_SaveToFile();

		m_pUserInterface->Shutdown();

		m_pThumbnailScene->Shutdown();
		m_pScene->Shutdown();


		m_pRenderer->Shutdown();
		m_pIbl_renderer->Shutdown();
		

		m_pExportManager->Shutdown();
		m_pTextureGenerationManager->Shutdown();
		m_pThumbnailManger->Shutdown();


		m_pMeshRegistry->Shutdown();
		m_pMaterialLibraryRegistry->Shutdown();
		m_pSkyboxAssetRegistry->Shutdown();


		m_pInputSystem->Shutdown();
		m_pDropHandler->Shutdown();

		m_pWindow->Shutdown();


		m_arena_persistent.arena_free_all();

		MNEMOSY_INFO("Mnemosy Application Closed");

		delete m_sInstance;
	}

	void* MnemosyEngine::arena_persistent_malloc(size_t size, size_t align)
	{
		MNEMOSY_ASSERT(m_arena_persistent.has_enough_memory(size),"Temporary Storage is out of memory");

		void* ptr = m_arena_persistent.arena_allocate(size, align);
		MNEMOSY_TRACE("Arena_Allocated {} bytes, Align {} , {} bytes left ", (int)size,(int)align, (int)m_arena_persistent.bytes_left());
		return ptr;
	}

} // !mnemosy