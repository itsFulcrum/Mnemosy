#include "Include/Application.h"

#include "Include/Gui/UserInterface.h"
#include "Include/Input/CameraInputController.h"
#include "Include/Input/SceneInputController.h"
#include "Include/GuiPanels/GuiPanelManager.h"

#include <string>

namespace mnemosy
{
	// private singleton stuff
	Application::Application() {	}
	Application* Application::m_sInstance = nullptr;


	// public
	Application& Application::GetInstance()
	{
		if (!m_sInstance)
		{
			m_sInstance = new Application();
		}

		return *m_sInstance;
	}

	void Application::Initialize()
	{
		m_mnemosyEngine.Initialize("Mnemosy Texture Library");



		m_cameraController = new input::CameraInputController();
		m_sceneInputController = new input::SceneInputController();


		m_pGuiPanelManager = new gui::GuiPanelManager();
	}

	void Application::Run()
	{
		m_mnemosyEngine.Run();
	}

	void Application::Shutdown()
	{
		delete m_cameraController;
		m_cameraController = nullptr;
		delete m_sceneInputController;
		m_sceneInputController = nullptr;


		delete m_pGuiPanelManager;
		m_pGuiPanelManager = nullptr;

		// shutdown engine last
		m_mnemosyEngine.Shutdown();

	}

	MnemosyEngine& Application::GetEngine()
	{
		return m_mnemosyEngine;
	}

} // mnemosy