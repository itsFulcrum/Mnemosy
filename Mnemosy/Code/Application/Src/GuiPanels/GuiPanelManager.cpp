#include "Include/GuiPanels/GuiPanelManager.h"

#include "Include/ApplicationConfig.h"
#include "Include/MnemosyEngine.h"
#include "Include/Core/Log.h"

#include "Include/Core/FileDirectories.h"
#include "Include/Gui/UserInterface.h"

#include "Include/GuiPanels/MainMenuBarGuiPanel.h"
#include "Include/GuiPanels/ViewportGuiPanel.h"
#include "Include/GuiPanels/SettingsGuiPanel.h"
#include "Include/GuiPanels/MaterialEditorGuiPanel.h"
#include "Include/GuiPanels/MaterialLibraryGuiPanel.h"
#include "Include/GuiPanels/DocumentationGuiPanel.h"
#include "Include/GuiPanels/ContentsGuiPanel.h"
#include "Include/GuiPanels/LogGuiPanel.h"


#include <filesystem>
#include <string>
#include <json.hpp>

namespace mnemosy::gui
{

	void GuiPanelManager::Init(){

		UserInterface& userInterface = MnemosyEngine::GetInstance().GetUserInterface();
		
		// Create and register ImGui panels

		m_pMainMenuBarPanel		= new MainMenuBarGuiPanel();
		m_pViewportPanel		= new ViewportGuiPanel();
		m_pSettingsPanel		= new SettingsGuiPanel();
		m_pMaterialEditorPanel	= new MaterialEditorGuiPanel();
		m_pMaterialLibraryPanel = new MaterialLibraryGuiPanel();
		m_pDocumentationPanel	= new DocumentationGuiPanel();
		m_pContentsPanel		= new ContentsGuiPanel();
		m_pLogPanel				= new LogGuiPanel();

		userInterface.RegisterMainMenuBarGuiPanel(*m_pMainMenuBarPanel);
		userInterface.RegisterGuiPanel(m_pViewportPanel);	
		userInterface.RegisterGuiPanel(m_pSettingsPanel);
		userInterface.RegisterGuiPanel(m_pMaterialEditorPanel);
		userInterface.RegisterGuiPanel(m_pMaterialLibraryPanel);
		userInterface.RegisterGuiPanel(m_pDocumentationPanel);
		userInterface.RegisterGuiPanel(m_pContentsPanel);
		userInterface.RegisterGuiPanel(m_pLogPanel);

		UserSettingsLoad(false);

#ifdef mnemosy_gui_showImGuiDemoWindow

	userInterface.show_demo_window = true;
#else
	userInterface.show_demo_window = false;
#endif // !mnemosy_gui_showDebugInfo
	}

	void GuiPanelManager::Shutdown(){

		UserSettingsSave();

		UserInterface& userInterface = MnemosyEngine::GetInstance().GetUserInterface();

		userInterface.UnregisterMainMenuBarGuiPanel();
		userInterface.UnregisterGuiPanel(m_pViewportPanel); 
		userInterface.UnregisterGuiPanel(m_pSettingsPanel);
		userInterface.UnregisterGuiPanel(m_pMaterialEditorPanel);
		userInterface.UnregisterGuiPanel(m_pMaterialLibraryPanel);
		userInterface.UnregisterGuiPanel(m_pDocumentationPanel);
		userInterface.UnregisterGuiPanel(m_pContentsPanel);
		userInterface.UnregisterGuiPanel(m_pLogPanel);

		delete m_pMainMenuBarPanel;
		delete m_pViewportPanel;
		delete m_pSettingsPanel;
		delete m_pMaterialEditorPanel;
		delete m_pMaterialLibraryPanel;
		delete m_pDocumentationPanel;
		delete m_pContentsPanel;
		delete m_pLogPanel;
	}



	void GuiPanelManager::UserSettingsLoad(bool restoreDefaults) {


		std::filesystem::path guiUserSettings_filepath = std::filesystem::path(MnemosyEngine::GetInstance().GetFileDirectories().GetUserSettingsPath() / std::filesystem::path("guiViewportsSettings.mnsydata"));

		bool success = false;

		flcrm::JsonSettings user;
		user.FilePrettyPrintSet(true);
		
		user.FileOpen(success,guiUserSettings_filepath, "Mnemosy Settings File", "This file stores user settings");
		if(!success){
			MNEMOSY_WARN("UserSettingsManager::SaveToFile: Failed to open user settings file: Msg: {}", user.ErrorStringLastGet());
		}

		if(restoreDefaults){

			// to restore defaults we simply delete everything then close file to save it and then subsequent read calls will return the defaults.
			user.FileWhipe(success);
			user.FileClose(success, guiUserSettings_filepath);
			user.FileOpen(success,guiUserSettings_filepath, "Mnemosy Settings File", "This file stores user settings");
		}


		// load gui panel states

		bool gp_documentation_Open		= user.ReadBool(success,"guiPanel_documentation_isOpen", true,true);
		bool gp_sceneSettings_Open		= user.ReadBool(success,"guiPanel_settings_isOpen", true,true);
		bool gp_materialLibrary_Open	= user.ReadBool(success,"guiPanel_materialLibrary_isOpen", true,true);
		bool gp_materialEditor_Open		= user.ReadBool(success,"guiPanel_materialEditor_isOpen", true,true);
		bool gp_viewport_Open			= user.ReadBool(success,"guiPanel_viewport_isOpen", true,true);
		bool gp_contents_Open			= user.ReadBool(success,"guiPanel_contents_isOpen", true,true);
		bool gp_log_Open				= user.ReadBool(success,"guiPanel_log_isOpen", true, true);
		
		// set gui panel states
		gui::UserInterface& userInterface = MnemosyEngine::GetInstance().GetUserInterface();

		userInterface.GetGuiPanel(gui::MNSY_GUI_PANEL_DOCUMENTATION).SetActive(gp_documentation_Open);
		userInterface.GetGuiPanel(gui::MNSY_GUI_PANEL_SETTINGS).SetActive(gp_sceneSettings_Open);
		userInterface.GetGuiPanel(gui::MNSY_GUI_PANEL_MATERIAL_LIBRARY).SetActive(gp_materialLibrary_Open);
		userInterface.GetGuiPanel(gui::MNSY_GUI_PANEL_MATERIAL_EDITOR).SetActive(gp_materialEditor_Open);
		userInterface.GetGuiPanel(gui::MNSY_GUI_PANEL_VIEWPORT).SetActive(gp_viewport_Open);
		userInterface.GetGuiPanel(gui::MNSY_GUI_PANEL_CONTENTS).SetActive(gp_contents_Open);
		userInterface.GetGuiPanel(gui::MNSY_GUI_PANEL_LOG).SetActive(gp_log_Open);
		

		float guiPanel_contents_buttonSize = user.ReadFloat(success,"guiPanel_contents_buttonSize", 128.0f,true);
		GetContentsPanel().ImageButtonSizeSet(guiPanel_contents_buttonSize);

		user.FileClose(success, guiUserSettings_filepath);

	}

	void GuiPanelManager::UserSettingsSave() {

		std::filesystem::path guiUserSettings_filepath = std::filesystem::path(MnemosyEngine::GetInstance().GetFileDirectories().GetUserSettingsPath() / std::filesystem::path("guiViewportsSettings.mnsydata"));


		bool success = false;

		flcrm::JsonSettings user;
		user.FilePrettyPrintSet(true);
		
		user.FileOpen(success,guiUserSettings_filepath, "Mnemosy Settings File", "This file stores user settings");
		if(!success){
			MNEMOSY_WARN("UserSettingsManager::SaveToFile: Failed to open user settings file: Msg: {}", user.ErrorStringLastGet());
		}

		// store wich viewports are visible
		gui::UserInterface& userInterface = MnemosyEngine::GetInstance().GetUserInterface();

		user.WriteBool(success,"guiPanel_documentation_isOpen", userInterface.IsGuiPanelVisible(gui::MNSY_GUI_PANEL_DOCUMENTATION));
		user.WriteBool(success,"guiPanel_settings_isOpen", userInterface.IsGuiPanelVisible(gui::MNSY_GUI_PANEL_SETTINGS));
		user.WriteBool(success,"guiPanel_materialLibrary_isOpen", userInterface.IsGuiPanelVisible(gui::MNSY_GUI_PANEL_MATERIAL_LIBRARY));
		user.WriteBool(success,"guiPanel_materialEditor_isOpen", userInterface.IsGuiPanelVisible(gui::MNSY_GUI_PANEL_MATERIAL_EDITOR));
		user.WriteBool(success,"guiPanel_viewport_isOpen", userInterface.IsGuiPanelVisible(gui::MNSY_GUI_PANEL_VIEWPORT));
		user.WriteBool(success,"guiPanel_contents_isOpen", userInterface.IsGuiPanelVisible(gui::MNSY_GUI_PANEL_CONTENTS));
		user.WriteBool(success, "guiPanel_log_isOpen", userInterface.IsGuiPanelVisible(gui::GuiPanelType::MNSY_GUI_PANEL_LOG));

		// TODO add more settings
		float guiPanel_contents_buttonSize = GetContentsPanel().ImageButtonSizeGet();
		user.WriteFloat(success,"guiPanel_contents_buttonSize", guiPanel_contents_buttonSize);

		user.FileClose(success, guiUserSettings_filepath);

	}
}