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
		
		if (!m_sInstance) {
			m_sInstance = new MnemosyEngine();
		}

		return *m_sInstance;
	}

	void MnemosyEngine::Initialize(const char* WindowTitle) {
		

		flcrm_log_assert(!m_isInitialized, "Engine is already initialized");
		
		
		m_logger.Init();

		MNEMOSY_INFO("Starting Mnemosy v{}.{}-{}", MNEMOSY_VERSION_MAJOR, MNEMOSY_VERSION_MINOR,MNEMOSY_VERSION_SUFFIX);


		m_arena_persistent.arena_init_allocate_buffer(1648); //1024000 = 1 mb memory block



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

		


		//MNEMOSY_TRACE("Initializing Subsystems");
		//  === !!!  order of initialization here matters !!!! ===

		m_pFileDirectories = arena_placement_new(core::FileDirectories);// need to come before scene and image base lighting renderer
		m_pFileDirectories->Init();

		//MNEMOSY_WARN("Init: FileDirectories");

		m_pWindow = arena_placement_new(core::Window);
		m_pWindow->Init(WindowTitle);

		#ifdef MNEMOSY_CONFIG_DISABLE_VSYNC
		m_pWindow->EnableVsync(false);
		#endif

		//MNEMOSY_WARN("Init: Window");
		
		// subsystems
		// Mnemosy::core
		m_pClock = arena_placement_new(core::Clock);
		m_pClock->Init();

		//MNEMOSY_WARN("Init: Clock");
		
		m_pDropHandler = arena_placement_new(core::DropHandler);
		m_pDropHandler->Init(m_pWindow->GetWindow());

		//MNEMOSY_WARN("Init: DropHandler");
		// mnemosy::systems
		m_pInputSystem = arena_placement_new(systems::InputSystem);
		m_pInputSystem->Init();

		//MNEMOSY_WARN("Init: InputSystem");

		m_pSkyboxAssetRegistry = arena_placement_new(systems::SkyboxAssetRegistry);
		m_pSkyboxAssetRegistry->Init();

		//MNEMOSY_WARN("Init: SkyReg");

		m_pThumbnailManager = arena_placement_new(systems::ThumbnailManager);
		m_pThumbnailManager->Init();

		//MNEMOSY_WARN("Init: ThumbnailMana");

		m_pTextureGenerationManager = arena_placement_new(systems::TextureGenerationManager);
		m_pTextureGenerationManager->Init();

		//MNEMOSY_WARN("Init: TexGenManag");

		m_pMaterialLibraryRegistry = arena_placement_new(systems::MaterialLibraryRegistry);
		m_pMaterialLibraryRegistry->Init();

		//MNEMOSY_WARN("Init: MatLibReg");

		m_pExportManager = arena_placement_new(systems::ExportManager);
		m_pExportManager->Init();

		//MNEMOSY_WARN("Init: ExportManag");

		m_pMeshRegistry = arena_placement_new(systems::MeshRegistry);
		m_pMeshRegistry->Init();

		//MNEMOSY_WARN("Init: MeshReg");

		// menmosy::graphcs
		//MNEMOSY_DEBUG("Initializing Renderer");
		m_pIbl_renderer =  arena_placement_new(graphics::ImageBasedLightingRenderer);
		m_pIbl_renderer->Init();

		//MNEMOSY_WARN("Init: Ibl_Rend");
		
		m_pRenderer = arena_placement_new(graphics::Renderer);
		m_pRenderer->Init();

		//MNEMOSY_WARN("Init: Render");

		//MNEMOSY_DEBUG("Loading Scenes");
		m_pScene = arena_placement_new(graphics::Scene);
		m_pScene->Init();
		//MNEMOSY_WARN("Init: Scene");

		m_pThumbnailScene = arena_placement_new(graphics::ThumbnailScene);
		m_pThumbnailScene->Init();

		//MNEMOSY_WARN("Init: ThumbScene");

		m_pUserInterface =  arena_placement_new(gui::UserInterface);
		m_pUserInterface->Init();
		//MNEMOSY_WARN("Init: UserInterface");

		m_pRenderer->SetPbrShaderBrdfLutUniforms();
		m_pRenderer->SetPbrShaderLightUniforms(m_pScene->GetLight());
		m_pRenderer->SetShaderSkyboxUniforms(m_pScene->userSceneSettings, m_pScene->GetSkybox());


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

			m_pThumbnailManager->Update();
			//m_pScene->Update();

			// Rendering
			m_pRenderer->HotReloadPbrShader(m_pClock->GetDeltaSeconds());
			m_pRenderer->RenderScene(*m_pScene, m_pMaterialLibraryRegistry->GetEntryTypeToRenderWith());
			m_pUserInterface->Render();

			glfwSwapBuffers(&m_pWindow->GetWindow());
			
		} // End of main loop

	}

	void MnemosyEngine::Shutdown() {	
		
		m_pMaterialLibraryRegistry->SaveCurrentSate();

		m_pUserInterface->Shutdown();

		m_pThumbnailScene->Shutdown();
		m_pScene->Shutdown();


		m_pRenderer->Shutdown();
		m_pIbl_renderer->Shutdown();
		

		m_pExportManager->Shutdown();
		m_pTextureGenerationManager->Shutdown();
		m_pThumbnailManager->Shutdown();


		m_pMeshRegistry->Shutdown();
		m_pMaterialLibraryRegistry->Shutdown();
		m_pSkyboxAssetRegistry->Shutdown();


		m_pInputSystem->Shutdown();
		m_pDropHandler->Shutdown();
		

		m_pFileDirectories->Shutdown();

		m_pWindow->Shutdown();


		m_arena_persistent.arena_free_all();

		MNEMOSY_INFO("Mnemosy Application Closed");

		m_logger.Shutdown();

		delete m_sInstance;
	}

	void* MnemosyEngine::arena_persistent_malloc(size_t size, size_t align) {

#ifdef MNEMOSY_CONFIG_DEBUG
		MNEMOSY_ASSERT(m_arena_persistent.arena_has_enough_memory(size),"Arena Storage is out of memory");
#endif // MNEMOSY_CONFIG_DEBUG

		void* ptr = m_arena_persistent.arena_allocate(size, align);

#ifdef MNEMOSY_CONFIG_DEBUG
		MNEMOSY_TRACE("Arena_Allocated {} bytes, Align {} , {} bytes left ", (int)size,(int)align, (int)m_arena_persistent.arena_get_bytes_left());
#endif // MNEMOSY_CONFIG_DEBUG
		
		
		return ptr;
	}

} // !mnemosy