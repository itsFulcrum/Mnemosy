#include "Include/GuiPanels/GuiPanelManager.h"

#include "Include/Application.h"
#include "Include/ApplicationConfig.h"

#include "Include/GuiPanels/MainMenuBarGuiPanel.h"
#include "Include/GuiPanels/ViewportGuiPanel.h"
#include "Include/GuiPanels/SettingsGuiPanel.h"
#include "Include/GuiPanels/MaterialEditorGuiPanel.h"
#include "Include/GuiPanels/MaterialLibraryGuiPanel.h"
#include "Include/GuiPanels/DocumentationGuiPanel.h"
#include "Include/GuiPanels/ContentsGuiPanel.h"


#include "Include/MnemosyEngine.h"
#include "Include/Systems/UserSettingsManager.h"
#include "Include/Gui/UserInterface.h"

namespace mnemosy::gui
{

	GuiPanelManager::GuiPanelManager() {

		UserInterface& userInterface = MnemosyEngine::GetInstance().GetUserInterface();
		
		// Create and register ImGui panels

		m_pMainMenuBarPanel		= new MainMenuBarGuiPanel();
		m_pViewportPanel		= new ViewportGuiPanel();
		m_pSettingsPanel		= new SettingsGuiPanel();
		m_pMaterialEditorPanel	= new MaterialEditorGuiPanel();
		m_pMaterialLibraryPanel = new MaterialLibraryGuiPanel();
		m_pDocumentationPanel	= new DocumentationGuiPanel();
		m_pContentsPanel		= new ContentsGuiPanel();

		userInterface.RegisterMainMenuBarGuiPanel(*m_pMainMenuBarPanel);
		userInterface.RegisterGuiPanel(m_pViewportPanel);	
		userInterface.RegisterGuiPanel(m_pSettingsPanel);
		userInterface.RegisterGuiPanel(m_pMaterialEditorPanel);
		userInterface.RegisterGuiPanel(m_pMaterialLibraryPanel);
		userInterface.RegisterGuiPanel(m_pDocumentationPanel);
		userInterface.RegisterGuiPanel(m_pContentsPanel);
		
		// load user settings after all panels have been initialized
		MnemosyEngine::GetInstance().GetUserSettingsManager().LoadUserSettings(false);

#ifdef mnemosy_gui_showImGuiDemoWindow

	userInterface.show_demo_window = true;
#else
	userInterface.show_demo_window = false;
#endif // !mnemosy_gui_showDebugInfo


	}

	GuiPanelManager::~GuiPanelManager() {

		// safe user settings before destroying the gui panels
		MnemosyEngine::GetInstance().GetUserSettingsManager().SaveToFile();

		UserInterface& userInterface = MnemosyEngine::GetInstance().GetUserInterface();

		userInterface.UnregisterMainMenuBarGuiPanel();
		userInterface.UnregisterGuiPanel(m_pViewportPanel); 
		userInterface.UnregisterGuiPanel(m_pSettingsPanel);
		userInterface.UnregisterGuiPanel(m_pMaterialEditorPanel);
		userInterface.UnregisterGuiPanel(m_pMaterialLibraryPanel);
		userInterface.UnregisterGuiPanel(m_pDocumentationPanel);
		userInterface.UnregisterGuiPanel(m_pContentsPanel);


		delete m_pMainMenuBarPanel;
		delete m_pViewportPanel;
		delete m_pSettingsPanel;
		delete m_pMaterialEditorPanel;
		delete m_pMaterialLibraryPanel;
		delete m_pDocumentationPanel;
		delete m_pContentsPanel;

	}
}