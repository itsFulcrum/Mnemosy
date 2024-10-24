#include "Include/Systems/GuiUserSettingsManager.h"

#include "Include/Application.h"
#include "Include/GuiPanels/GuiPanelManager.h"
#include "Include/GuiPanels/ContentsGuiPanel.h"


#include "Include/MnemosyEngine.h"


#include "Include/Core/Log.h"
#include "Include/Core/FileDirectories.h"

#include <json.hpp>

#include "Include/Gui/UserInterface.h"
#include "Include/Gui/GuiPanel.h"


#include <filesystem>
#include <string>


namespace mnemosy::systems {

	GuiUserSettingsManager::GuiUserSettingsManager() {

	}

	GuiUserSettingsManager::~GuiUserSettingsManager() 
	{	}

	void GuiUserSettingsManager::UserSettingsLoad(bool restoreDefaults) {

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

		
		// set gui panel states
		gui::UserInterface& userInterface = MnemosyEngine::GetInstance().GetUserInterface();

		userInterface.GetGuiPanel(gui::MNSY_GUI_PANEL_DOCUMENTATION).SetActive(gp_documentation_Open);
		userInterface.GetGuiPanel(gui::MNSY_GUI_PANEL_SETTINGS).SetActive(gp_sceneSettings_Open);
		userInterface.GetGuiPanel(gui::MNSY_GUI_PANEL_MATERIAL_LIBRARY).SetActive(gp_materialLibrary_Open);
		userInterface.GetGuiPanel(gui::MNSY_GUI_PANEL_MATERIAL_EDITOR).SetActive(gp_materialEditor_Open);
		userInterface.GetGuiPanel(gui::MNSY_GUI_PANEL_VIEWPORT).SetActive(gp_viewport_Open);
		userInterface.GetGuiPanel(gui::MNSY_GUI_PANEL_CONTENTS).SetActive(gp_contents_Open);


		float guiPanel_contents_buttonSize = user.ReadFloat(success,"guiPanel_contents_buttonSize", 128.0f,true);
		Application::GetInstance().GetGuiPanelManager().GetContentsPanel().ImageButtonSizeSet(guiPanel_contents_buttonSize);

		user.FileClose(success, guiUserSettings_filepath);

	}

	void GuiUserSettingsManager::UserSettingsSave() {

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

		// TODO add more settings
		float guiPanel_contents_buttonSize = Application::GetInstance().GetGuiPanelManager().GetContentsPanel().ImageButtonSizeGet();
		user.WriteFloat(success,"guiPanel_contents_buttonSize", guiPanel_contents_buttonSize);

		user.FileClose(success, guiUserSettings_filepath);

	}



} // ! namespace mnemosy::systems