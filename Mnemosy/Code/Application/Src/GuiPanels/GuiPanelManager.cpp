#include "Application/Include/GuiPanels/GuiPanelManager.h"

#include "Engine/Include/MnemosyEngine.h"
#include "Engine/Include/Gui/UserInterface.h"

#include "Application/Include/Application.h"
#include "Application/Include/GuiPanels/ViewportGuiPanel.h"
#include "Application/Include/GuiPanels/SceneSettingsGuiPanel.h"
#include "Application/Include/GuiPanels/MaterialEditorGuiPanel.h"

namespace mnemosy::gui
{
	GuiPanelManager::GuiPanelManager()
	{
		UserInterface& userInterface = ENGINE_INSTANCE().GetUserInterface();
		// gui panels
		m_pViewportPanel = new gui::ViewportGuiPanel();
		userInterface.RegisterGuiPanel(m_pViewportPanel);
		m_pSceneSettingsPanel = new gui::SceneSettingsGuiPanel();
		userInterface.RegisterGuiPanel(m_pSceneSettingsPanel);
		m_pMaterialEditorPanel = new gui::MaterialEditorGuiPanel();
		userInterface.RegisterGuiPanel(m_pMaterialEditorPanel);


	}

	GuiPanelManager::~GuiPanelManager()
	{
		UserInterface& userInterface = ENGINE_INSTANCE().GetUserInterface();

		userInterface.UnregisterGuiPanel(m_pViewportPanel);
		delete m_pViewportPanel;
		m_pViewportPanel = nullptr;
		
		userInterface.UnregisterGuiPanel(m_pSceneSettingsPanel);
		delete m_pSceneSettingsPanel;
		m_pSceneSettingsPanel = nullptr;

		userInterface.UnregisterGuiPanel(m_pMaterialEditorPanel);
		delete m_pMaterialEditorPanel;
		m_pMaterialEditorPanel = nullptr;
	}

}