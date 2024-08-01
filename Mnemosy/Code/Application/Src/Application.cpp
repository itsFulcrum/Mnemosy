#include "Include/Application.h"

#include "Include/MnemosyEngine.h"

#include "Include/Core/Log.h"
#include "Include/Core/Clock.h"
#include "Include/Input/CameraInputController.h"
#include "Include/Input/SceneInputController.h"
#include "Include/GuiPanels/GuiPanelManager.h"

namespace mnemosy
{
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

	Application::~Application()
	{	}

	void Application::Initialize()	{

		m_mnemosyEngine.Initialize("Mnemosy");

		m_pCameraController = new input::CameraInputController();
		m_pSceneInputController = new input::SceneInputController();
		m_pGuiPanelManager = new gui::GuiPanelManager();

		double applicationLoadTime = m_mnemosyEngine.GetClock().GetTimeSinceLaunch();
		MNEMOSY_INFO("Mnemosy Application Initialized: {} Seconds", applicationLoadTime);
	}

	void Application::Run() {

		m_mnemosyEngine.Run();
	}

	void Application::Shutdown() {

		delete m_pCameraController;
		delete m_pSceneInputController;
		delete m_pGuiPanelManager;

		// shutdown engine last
		m_mnemosyEngine.Shutdown();

		delete m_sInstance;
	}

} // mnemosy