#include "Include/Systems/UserSettingsManager.h"

#include "Include/MnemosyEngine.h"

#include "Include/Core/Log.h"
#include "Include/Core/FileDirectories.h"

#include "Include/Gui/UserInterface.h"
#include "Include/Gui/GuiPanel.h"


#include <string>
#include <fstream>
#include <nlohmann/json.hpp>


namespace mnemosy::systems {

	UserSettingsManager::UserSettingsManager()
	{
		m_userSettingsDataFilePath = std::filesystem::path(MnemosyEngine::GetInstance().GetFileDirectories().GetDataPath() / std::filesystem::path("UserSettingsData.mnsydata"));
	}

	UserSettingsManager::~UserSettingsManager() {

		
	}



	void UserSettingsManager::LoadUserSettings() {

		namespace fs = std::filesystem;
		using namespace nlohmann;

		fs::directory_entry dataFile = fs::directory_entry(m_userSettingsDataFilePath);
		if (!CheckDataFile(dataFile)) {
			return;
		}


		json json_userSettings;
		{

			std::ifstream dataFileStream;
			dataFileStream.open(m_userSettingsDataFilePath);
			json json_readFile;
			try {
				json_readFile = json::parse(dataFileStream);
			}
			catch (json::parse_error err) {
				MNEMOSY_ERROR("UserSettingsManager::LoadUserSettings: Error Parsing File. Message: {}", err.what());
				return;
			}



			json_userSettings = json_readFile["3_UserSettings"].get<json>();
			dataFileStream.close();

		}

		// load gui panel states
		{
			gui::UserInterface& userInterface = MnemosyEngine::GetInstance().GetUserInterface();

			json json_guiPanels = json_userSettings["guiPanelStates"].get<json>();


			if (!json_guiPanels["gp_documentation_Open"].get<bool>())	{ userInterface.GetGuiPanel(gui::MNSY_GUI_PANEL_DOCUMENTATION).Deactivate(); }
			if (!json_guiPanels["gp_globalSettings_Open"].get<bool>())	{ userInterface.GetGuiPanel(gui::MNSY_GUI_PANEL_GLOBAL_SETTINGS).Deactivate();}
			if (!json_guiPanels["gp_sceneSettings_Open"].get<bool>())	{ userInterface.GetGuiPanel(gui::MNSY_GUI_PANEL_SCENE_SETTINGS).Deactivate(); }
			if (!json_guiPanels["gp_materialLibrary_Open"].get<bool>()) { userInterface.GetGuiPanel(gui::MNSY_GUI_PANEL_MATERIAL_LIBRARY).Deactivate(); }
			if (!json_guiPanels["gp_materialEditor_Open"].get<bool>())	{ userInterface.GetGuiPanel(gui::MNSY_GUI_PANEL_MATERIAL_EDITOR).Deactivate(); }
			if (!json_guiPanels["gp_viewport_Open"].get<bool>())		{ userInterface.GetGuiPanel(gui::MNSY_GUI_PANEL_VIEWPORT).Deactivate(); }

		}
	}

	void UserSettingsManager::SaveToFile() {

		using namespace nlohmann;


		json json_userSettingsTopLevel;
		json_userSettingsTopLevel["1_Mnemosy_Data_File"] = "UserSettingsData";

		json json_headerInfo;
		json_headerInfo["Description"] = "This files stores user settings";

		json_userSettingsTopLevel["2_Header_Info"] = json_headerInfo;


		json json_userSettings; // holds sub jsons for all the settings


		// store what viewports are visible
		json json_guiPanelStates;
		{
			gui::UserInterface& userInterface = MnemosyEngine::GetInstance().GetUserInterface();
		
			json_guiPanelStates["gp_viewport_Open"]				= userInterface.IsGuiPanelVisible(gui::MNSY_GUI_PANEL_VIEWPORT);
			json_guiPanelStates["gp_globalSettings_Open"]		= userInterface.IsGuiPanelVisible(gui::MNSY_GUI_PANEL_GLOBAL_SETTINGS);
			json_guiPanelStates["gp_sceneSettings_Open"]		= userInterface.IsGuiPanelVisible(gui::MNSY_GUI_PANEL_SCENE_SETTINGS);
			json_guiPanelStates["gp_materialLibrary_Open"]		= userInterface.IsGuiPanelVisible(gui::MNSY_GUI_PANEL_MATERIAL_LIBRARY);
			json_guiPanelStates["gp_materialEditor_Open"]		= userInterface.IsGuiPanelVisible(gui::MNSY_GUI_PANEL_MATERIAL_EDITOR);
			json_guiPanelStates["gp_documentation_Open"]		= userInterface.IsGuiPanelVisible(gui::MNSY_GUI_PANEL_DOCUMENTATION);

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