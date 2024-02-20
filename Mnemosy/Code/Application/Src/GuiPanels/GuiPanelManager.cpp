#include "Application/Include/GuiPanels/GuiPanelManager.h"

#include "Engine/Include/MnemosyEngine.h"
#include "Engine/Include/Gui/UserInterface.h"

#include "Application/Include/Application.h"
#include "Application/Include/GuiPanels/ViewportGuiPanel.h"
#include "Application/Include/GuiPanels/SceneSettingsGuiPanel.h"

namespace mnemosy::gui
{
	GuiPanelManager::GuiPanelManager()
	{
		UserInterface& userInterface = ENGINE_INSTANCE().GetUserInterface();
		// gui panels
		m_viewportPanel = new gui::ViewportGuiPanel();
		userInterface.RegisterGuiPanel(m_viewportPanel);
		m_sceneSettingsPanel = new gui::SceneSettingsGuiPanel();
		userInterface.RegisterGuiPanel(m_sceneSettingsPanel);
	}

	GuiPanelManager::~GuiPanelManager()
	{
		UserInterface& userInterface = ENGINE_INSTANCE().GetUserInterface();

		userInterface.UnregisterGuiPanel(m_viewportPanel);
		delete m_viewportPanel;
		m_viewportPanel = nullptr;
		
		userInterface.UnregisterGuiPanel(m_sceneSettingsPanel);
		delete m_sceneSettingsPanel;
		m_sceneSettingsPanel = nullptr;

	}

}