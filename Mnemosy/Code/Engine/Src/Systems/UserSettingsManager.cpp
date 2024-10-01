#include "Include/Systems/UserSettingsManager.h"

#include "Include/MnemosyEngine.h"

#include "Include/Core/Log.h"
#include "Include/Core/FileDirectories.h"

#include "Include/Gui/UserInterface.h"
#include "Include/Gui/GuiPanel.h"

#include "Include/Core/JsonSettings.h"


#include <vector>
#include <string>
#include <fstream>
#include <nlohmann/json.hpp>


namespace mnemosy::systems {

	UserSettingsManager::UserSettingsManager() {

		m_userSettingsDataFilePath = std::filesystem::path(MnemosyEngine::GetInstance().GetFileDirectories().GetDataPath() / std::filesystem::path("UserSettingsData.mnsydata"));

	}

	UserSettingsManager::~UserSettingsManager() 
	{	}

	void UserSettingsManager::LoadUserSettings(bool useDefaultFile) {

		namespace fs = std::filesystem;
		namespace js = nlohmann;

		fs::path dataFilePath = m_userSettingsDataFilePath;

		if (useDefaultFile) {
			dataFilePath = fs::path(MnemosyEngine::GetInstance().GetFileDirectories().GetDataPath() / fs::path("UserSettingsData_default.mnsydata"));
		}

		fs::directory_entry dataFile = fs::directory_entry(dataFilePath);

		if (!CheckDataFile(dataFile)) {
			return;
		}


		std::ifstream dataFileStream;
		dataFileStream.open(dataFilePath);
		js::json json_readFile;

		try {
			json_readFile = js::json::parse(dataFileStream);
		}
		catch (js::json::parse_error error) {
			MNEMOSY_ERROR("UserSettingsManager::LoadUserSettings: Error Parsing File. Message: {}", error.what());
			return;
		}

		dataFileStream.close();


		js::json json_userSettings = json_readFile["3_UserSettings"].get<js::json>();


		

		// load gui panel states
		{
			gui::UserInterface& userInterface = MnemosyEngine::GetInstance().GetUserInterface();

			js::json json_guiPanels = json_userSettings["guiPanelStates"].get<js::json>();


			bool gp_documentation_Open		= json_guiPanels["gp_documentation_Open"].get<bool>();
			bool gp_sceneSettings_Open		= json_guiPanels["gp_settings_Open"].get<bool>();
			bool gp_materialLibrary_Open	= json_guiPanels["gp_materialLibrary_Open"].get<bool>();
			bool gp_materialEditor_Open		= json_guiPanels["gp_materialEditor_Open"].get<bool>();
			bool gp_viewport_Open			= json_guiPanels["gp_viewport_Open"].get<bool>();
			bool gp_contents_Open			= json_guiPanels["gp_contents_Open"].get<bool>();


			userInterface.GetGuiPanel(gui::MNSY_GUI_PANEL_DOCUMENTATION).SetActive(gp_documentation_Open);
			userInterface.GetGuiPanel(gui::MNSY_GUI_PANEL_SETTINGS).SetActive(gp_sceneSettings_Open);
			userInterface.GetGuiPanel(gui::MNSY_GUI_PANEL_MATERIAL_LIBRARY).SetActive(gp_materialLibrary_Open);
			userInterface.GetGuiPanel(gui::MNSY_GUI_PANEL_MATERIAL_EDITOR).SetActive(gp_materialEditor_Open);
			userInterface.GetGuiPanel(gui::MNSY_GUI_PANEL_VIEWPORT).SetActive(gp_viewport_Open);
			userInterface.GetGuiPanel(gui::MNSY_GUI_PANEL_CONTENTS).SetActive(gp_contents_Open);
		}


		if (useDefaultFile) {

			SaveToFile();
			MNEMOSY_INFO("Restored Default Settings");
		}
	}

	void UserSettingsManager::SaveToFile() {

		namespace fs = std::filesystem;
		namespace js = nlohmann;


		js::json json_userSettingsTopLevel;
		json_userSettingsTopLevel["1_Mnemosy_Data_File"] = "UserSettingsData";

		js::json json_headerInfo;
		json_headerInfo["Description"] = "This files stores user settings";

		json_userSettingsTopLevel["2_Header_Info"] = json_headerInfo;


		js::json json_userSettings; // holds sub jsons for all the settings


		// store what viewports are visible
		js::json json_guiPanelStates;
		{
			gui::UserInterface& userInterface = MnemosyEngine::GetInstance().GetUserInterface();
		
			json_guiPanelStates["gp_viewport_Open"]				= userInterface.IsGuiPanelVisible(gui::MNSY_GUI_PANEL_VIEWPORT);
			json_guiPanelStates["gp_settings_Open"]				= userInterface.IsGuiPanelVisible(gui::MNSY_GUI_PANEL_SETTINGS);
			json_guiPanelStates["gp_materialLibrary_Open"]		= userInterface.IsGuiPanelVisible(gui::MNSY_GUI_PANEL_MATERIAL_LIBRARY);
			json_guiPanelStates["gp_materialEditor_Open"]		= userInterface.IsGuiPanelVisible(gui::MNSY_GUI_PANEL_MATERIAL_EDITOR);
			json_guiPanelStates["gp_documentation_Open"]		= userInterface.IsGuiPanelVisible(gui::MNSY_GUI_PANEL_DOCUMENTATION);
			json_guiPanelStates["gp_contents_Open"]				= userInterface.IsGuiPanelVisible(gui::MNSY_GUI_PANEL_CONTENTS);
		}

		json_userSettings["guiPanelStates"] = json_guiPanelStates;

		// TODO add more settings

		json_userSettingsTopLevel["3_UserSettings"] = json_userSettings;


		//  write to file
		fs::directory_entry dataFile = fs::directory_entry(m_userSettingsDataFilePath);


		CheckDataFile(dataFile);


		std::ofstream dataFileStream;

		bool prettyPrintJson = true;

		dataFileStream.open(m_userSettingsDataFilePath);

		if (prettyPrintJson)
			dataFileStream << json_userSettingsTopLevel.dump(4);
		else
			dataFileStream << json_userSettingsTopLevel.dump(-1);


		dataFileStream.close();
	}


	bool UserSettingsManager::CheckDataFile(std::filesystem::directory_entry& fileDirectoryEntry) {

		std::string pathToDataFileString = fileDirectoryEntry.path().generic_string();


		if (!fileDirectoryEntry.exists())
		{
			MNEMOSY_ERROR("UserSettingsManager::CheckDataFile: File did Not Exist: {} \nCreating new file", pathToDataFileString);

			std::ofstream file;
			file.open(pathToDataFileString);
			file << "";
			file.close();

			return false;
		}

		if (!fileDirectoryEntry.is_regular_file())
		{
			MNEMOSY_ERROR("UserSettingsManager::CheckDataFile: File is not a regular file: {} \nCreating new file", pathToDataFileString);
			// maybe need to delete unregular file first idk should never happen anyhow
			std::ofstream file;
			file.open(pathToDataFileString);
			file << "";
			file.close();
			return false;
		}

		return true;
	}



} // namespace mnemosy::systems