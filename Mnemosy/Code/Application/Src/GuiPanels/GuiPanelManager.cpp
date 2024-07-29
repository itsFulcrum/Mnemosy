#include "Include/GuiPanels/GuiPanelManager.h"
#include "Include/GuiPanels/MainMenuBarGuiPanel.h"
#include "Include/GuiPanels/ViewportGuiPanel.h"
#include "Include/GuiPanels/SceneSettingsGuiPanel.h"
#include "Include/GuiPanels/MaterialEditorGuiPanel.h"
#include "Include/GuiPanels/MaterialLibraryGuiPanel.h"
#include "Include/GuiPanels/DocumentationGuiPanel.h"

#include "Include/Application.h"
#include "Include/ApplicationConfig.h"

#include "Include/MnemosyEngine.h"
#include "Include/Gui/UserInterface.h"
#include "Include/Systems/UserSettingsManager.h"


namespace mnemosy::gui
{
	GuiPanelManager::GuiPanelManager()
	{
		UserInterface& userInterface = MnemosyEngine::GetInstance().GetUserInterface();
		// gui panels
		m_pMainMenuBarPanel = new MainMenuBarGuiPanel();
		userInterface.RegisterMainMenuBarGuiPanel(*m_pMainMenuBarPanel);
		
		m_pViewportPanel = new ViewportGuiPanel();
		userInterface.RegisterGuiPanel(m_pViewportPanel);	
		
		m_pSceneSettingsPanel = new SceneSettingsGuiPanel();
		userInterface.RegisterGuiPanel(m_pSceneSettingsPanel);
		
		m_pMaterialEditorPanel = new MaterialEditorGuiPanel();
		userInterface.RegisterGuiPanel(m_pMaterialEditorPanel);
		
		m_pMaterialLibraryPanel = new MaterialLibraryGuiPanel();
		userInterface.RegisterGuiPanel(m_pMaterialLibraryPanel);

		m_pDocumentationPanel = new DocumentationGuiPanel();
		userInterface.RegisterGuiPanel(m_pDocumentationPanel);

		
		// load user settings after all panels have been initialized
		MnemosyEngine::GetInstance().GetUserSettingsManager().LoadUserSettings(false);

#ifdef mnemosy_gui_showImGuiDemoWindow

	userInterface.show_demo_window = true;
#else
	userInterface.show_demo_window = false;
#endif // !mnemosy_gui_showDebugInfo


	}

	GuiPanelManager::~GuiPanelManager()
	{
		// safe user settings before destroying the gui panels
		MnemosyEngine::GetInstance().GetUserSettingsManager().SaveToFile();

		UserInterface& userInterface = MnemosyEngine::GetInstance().GetUserInterface();

		userInterface.UnregisterMainMenuBarGuiPanel();
		delete m_pMainMenuBarPanel;
		m_pMainMenuBarPanel = nullptr;

		userInterface.UnregisterGuiPanel(m_pViewportPanel);
		delete m_pViewportPanel;
		m_pViewportPanel = nullptr;

		userInterface.UnregisterGuiPanel(m_pSceneSettingsPanel);
		delete m_pSceneSettingsPanel;
		m_pSceneSettingsPanel = nullptr;

		userInterface.UnregisterGuiPanel(m_pMaterialEditorPanel);
		delete m_pMaterialEditorPanel;
		m_pMaterialEditorPanel = nullptr;

		userInterface.UnregisterGuiPanel(m_pMaterialLibraryPanel);
		delete m_pMaterialLibraryPanel;
		m_pMaterialLibraryPanel = nullptr;

		userInterface.UnregisterGuiPanel(m_pDocumentationPanel);
		delete m_pDocumentationPanel;
		m_pDocumentationPanel = nullptr;

	}
}