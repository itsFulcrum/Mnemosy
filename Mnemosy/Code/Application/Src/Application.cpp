#include "Include/Application.h"

#include "Include/MnemosyEngine.h"
#include "Include/MnemosyConfig.h"

#include "Include/Core/Log.h"
#include "Include/Core/Clock.h"

#include "Include/Input/CameraInputController.h"
#include "Include/GuiPanels/GuiPanelManager.h"

#include <string>

namespace mnemosy {
	
	// private
	Application::Application()
		: m_mnemosyEngine{MnemosyEngine::GetInstance()}
	
	{	}
	Application* Application::m_sInstance = nullptr;


	// public
	Application& Application::GetInstance() {
		if (!m_sInstance)
		{
			m_sInstance = new Application();
		}

		return *m_sInstance;
	}

	void Application::Initialize()	{

		std::string windowTitle = "Mnemosy v" + std::to_string(MNEMOSY_VERSION_MAJOR) + "." + std::to_string(MNEMOSY_VERSION_MINOR) + "-" + MNEMOSY_VERSION_SUFFIX;
		m_mnemosyEngine.Initialize(windowTitle.c_str());


		m_pCameraController = (input::CameraInputController*)malloc(sizeof(input::CameraInputController)); // ccccccccceee styyyyyle
		m_pCameraController->Init();

		m_pGuiPanelManager = (gui::GuiPanelManager*)malloc(sizeof(gui::GuiPanelManager));
		m_pGuiPanelManager->Init();


		MNEMOSY_INFO("Mnemosy Application Initialized: {} Seconds", m_mnemosyEngine.GetClock().GetTimeSinceLaunch());
	}

	void Application::Run() {

		m_mnemosyEngine.Run();
	}

	void Application::Shutdown() {

		free(m_pCameraController);
		
		m_pGuiPanelManager->Shutdown();
		free(m_pGuiPanelManager);
		
		// shutdown engine last
		m_mnemosyEngine.Shutdown();

		delete m_sInstance;
	}

} // mnemosy