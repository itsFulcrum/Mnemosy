#include "Include/Application.h"

#include "Include/MnemosyEngine.h"
#include "Include/MnemosyConfig.h"

#include "Include/Core/Log.h"
#include "Include/Core/Clock.h"
#include "Include/Input/CameraInputController.h"
#include "Include/Input/SceneInputController.h"
#include "Include/GuiPanels/GuiPanelManager.h"
#include "Include/Systems/GuiUserSettingsManager.h"


#include <string>

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

		


		std::string windowTitle = "Mnemosy v" + std::to_string(MNEMOSY_VERSION_MAJOR) + "." + std::to_string(MNEMOSY_VERSION_MINOR) + "-" + MNEMOSY_VERSION_SUFFIX;
		m_mnemosyEngine.Initialize(windowTitle.c_str());

		m_pCameraController = new input::CameraInputController();
		m_pSceneInputController = new input::SceneInputController();
		m_pGuiPanelManager = new gui::GuiPanelManager();
		m_pGuiUserSettingsManager = new systems::GuiUserSettingsManager();

		m_pGuiUserSettingsManager->UserSettingsLoad(false); // load after panels and ui is initialized


		double applicationLoadTime = m_mnemosyEngine.GetClock().GetTimeSinceLaunch();
		MNEMOSY_INFO("Mnemosy Application Initialized: {} Seconds", applicationLoadTime);
	}

	void Application::Run() {

		m_mnemosyEngine.Run();
	}

	void Application::Shutdown() {

		m_pGuiUserSettingsManager->UserSettingsSave(); // save before destroying window and stuff

		delete m_pCameraController;
		delete m_pSceneInputController;
		delete m_pGuiPanelManager;
		delete m_pGuiUserSettingsManager;
		// shutdown engine last
		m_mnemosyEngine.Shutdown();

		delete m_sInstance;
	}

} // mnemosy