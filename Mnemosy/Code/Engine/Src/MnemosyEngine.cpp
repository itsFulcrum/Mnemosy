#include "Engine/Include/MnemosyEngine.h"
#include "Engine/Include/MnemosyConfig.h"

#include <GLFW/glfw3.h>

#include "Engine/Include/Core/Window.h"
#include "Engine/Include/Core/Logger.h"
#include "Engine/Include/Core/Log.h"
#include "Engine/Include/Core/Clock.h"

#include "Engine/Include/Systems/Input/InputSystem.h"

#include "Engine/Include/Graphics/Material.h"
#include "Engine/Include/Graphics/Renderer.h"
#include "Engine/Include/Graphics/Skybox.h"
#include "Engine/Include/Graphics/RenderMesh.h"
#include "Engine/Include/Graphics/Camera.h"
#include "Engine/Include/Graphics/ImageBasedLightingRenderer.h"
#include "Engine/Include/Graphics/Light.h"
#include "Engine/Include/Graphics/Scene.h"

#include "Engine/Include/Gui/UserInterface.h"

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
		// subsystems
		m_clock = std::make_unique<core::Clock>();


		//MNEMOSY_TRACE("Clock Initialized");
		m_pInputSystem = std::make_unique<systems::InputSystem>();
		//MNEMOSY_TRACE("InputSystem Initialized");
		m_pIbl_renderer = std::make_unique<graphics::ImageBasedLightingRenderer>();
		//MNEMOSY_TRACE("ibl_Renderer Initialized");
		m_pRenderer = std::make_unique<graphics::Renderer>();
		//MNEMOSY_TRACE("Renderer Initialized");

		m_pScene = std::make_unique<graphics::Scene>();
		//MNEMOSY_TRACE("Scene Initialized");



		m_pUserInterface = std::make_unique<gui::UserInterface>();

		
		m_pRenderer->SetPbrShaderBrdfLutUniforms();
		m_pRenderer->SetPbrShaderLightUniforms();
		m_pRenderer->SetShaderSkyboxUniforms();

		m_clock->capDeltaTime = true;

		m_isInitialized = true;

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


	// private



}