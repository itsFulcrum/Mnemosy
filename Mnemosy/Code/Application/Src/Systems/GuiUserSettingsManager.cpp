#include "Include/Systems/GuiUserSettingsManager.h"

#include "Include/Application.h"
#include "Include/GuiPanels/GuiPanelManager.h"
#include "Include/GuiPanels/ContentsGuiPanel.h"


#include "Include/MnemosyEngine.h"


#include "Include/Core/Log.h"
#include "Include/Core/FileDirectories.h"

#include "Include/Core/JsonSettings.h"

#include "Include/Gui/UserInterface.h"
#include "Include/Gui/GuiPanel.h"


//#include <vector>
#include <string>


namespace mnemosy::systems {

	GuiUserSettingsManager::GuiUserSettingsManager() {

		m_userSettingsDataFilePath = std::filesystem::path(MnemosyEngine::GetInstance().GetFileDirectories().GetDataPath() / std::filesystem::path("GuiUserSettings.mnsydata"));

	}

	GuiUserSettingsManager::~GuiUserSettingsManager() 
	{	}

	void GuiUserSettingsManager::UserSettingsLoad(bool restoreDefaults) {


		bool success = false;

		core::JsonSettings user;
		user.SettingsFilePrettyPrintSet(true);
		
		user.SettingsFileOpen(success,m_userSettingsDataFilePath, "Mnemosy Settings File", "This file stores user settings");
		if(!success){
			MNEMOSY_WARN("UserSettingsManager::SaveToFile: Failed to open user settings file: Msg: {}", user.ErrorStringLastGet());
		}

		if(restoreDefaults){

			// to restore defaults we simply delete everything then close file to save it and then subsequent read calls will return the defaults.
			user.SettingsFileWhipe(success);
			user.SettingsFileClose(success, m_userSettingsDataFilePath);
			user.SettingsFileOpen(success,m_userSettingsDataFilePath, "Mnemosy Settings File", "This file stores user settings");
		}


		// load gui panel states

		bool gp_documentation_Open		= user.SettingReadBool(success,"guiPanel_documentation_isOpen", true);
		bool gp_sceneSettings_Open		= user.SettingReadBool(success,"guiPanel_settings_isOpen", true);
		bool gp_materialLibrary_Open	= user.SettingReadBool(success,"guiPanel_materialLibrary_isOpen", true);
		bool gp_materialEditor_Open		= user.SettingReadBool(success,"guiPanel_materialEditor_isOpen", true);
		bool gp_viewport_Open			= user.SettingReadBool(success,"guiPanel_viewport_isOpen", true);
		bool gp_contents_Open			= user.SettingReadBool(success,"guiPanel_contents_isOpen", true);

		
		// set gui panel states
		gui::UserInterface& userInterface = MnemosyEngine::GetInstance().GetUserInterface();

		userInterface.GetGuiPanel(gui::MNSY_GUI_PANEL_DOCUMENTATION).SetActive(gp_documentation_Open);
		userInterface.GetGuiPanel(gui::MNSY_GUI_PANEL_SETTINGS).SetActive(gp_sceneSettings_Open);
		userInterface.GetGuiPanel(gui::MNSY_GUI_PANEL_MATERIAL_LIBRARY).SetActive(gp_materialLibrary_Open);
		userInterface.GetGuiPanel(gui::MNSY_GUI_PANEL_MATERIAL_EDITOR).SetActive(gp_materialEditor_Open);
		userInterface.GetGuiPanel(gui::MNSY_GUI_PANEL_VIEWPORT).SetActive(gp_viewport_Open);
		userInterface.GetGuiPanel(gui::MNSY_GUI_PANEL_CONTENTS).SetActive(gp_contents_Open);


		float guiPanel_contents_buttonSize = user.SettingReadFloat(success,"guiPanel_contents_buttonSize", 128.0f);
		Application::GetInstance().GetGuiPanelManager().GetContentsPanel().ImageButtonSizeSet(guiPanel_contents_buttonSize);

		user.SettingsFileClose(success, m_userSettingsDataFilePath);

	}

	void GuiUserSettingsManager::UserSettingsSave() {


		bool success = false;

		core::JsonSettings user;
		user.SettingsFilePrettyPrintSet(true);
		
		user.SettingsFileOpen(success,m_userSettingsDataFilePath, "Mnemosy Settings File", "This file stores user settings");
		if(!success){
			MNEMOSY_WARN("UserSettingsManager::SaveToFile: Failed to open user settings file: Msg: {}", user.ErrorStringLastGet());
		}

		// store wich viewports are visible
		gui::UserInterface& userInterface = MnemosyEngine::GetInstance().GetUserInterface();

		user.SettingWriteBool(success,"guiPanel_documentation_isOpen", userInterface.IsGuiPanelVisible(gui::MNSY_GUI_PANEL_DOCUMENTATION));
		user.SettingWriteBool(success,"guiPanel_settings_isOpen", userInterface.IsGuiPanelVisible(gui::MNSY_GUI_PANEL_SETTINGS));
		user.SettingWriteBool(success,"guiPanel_materialLibrary_isOpen", userInterface.IsGuiPanelVisible(gui::MNSY_GUI_PANEL_MATERIAL_LIBRARY));
		user.SettingWriteBool(success,"guiPanel_materialEditor_isOpen", userInterface.IsGuiPanelVisible(gui::MNSY_GUI_PANEL_MATERIAL_EDITOR));
		user.SettingWriteBool(success,"guiPanel_viewport_isOpen", userInterface.IsGuiPanelVisible(gui::MNSY_GUI_PANEL_VIEWPORT));
		user.SettingWriteBool(success,"guiPanel_contents_isOpen", userInterface.IsGuiPanelVisible(gui::MNSY_GUI_PANEL_CONTENTS));

		// TODO add more settings
		float guiPanel_contents_buttonSize = Application::GetInstance().GetGuiPanelManager().GetContentsPanel().ImageButtonSizeGet();
		user.SettingWriteFloat(success,"guiPanel_contents_buttonSize", guiPanel_contents_buttonSize);

		user.SettingsFileClose(success, m_userSettingsDataFilePath);

	}



} // ! namespace mnemosy::systems