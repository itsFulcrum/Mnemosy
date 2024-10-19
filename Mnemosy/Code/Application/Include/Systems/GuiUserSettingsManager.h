#ifndef GUI_USER_SETTINGS_MANAGER_H
#define GUI_USER_SETTINGS_MANAGER_H

namespace mnemosy::systems {

	class GuiUserSettingsManager {

	public:
		GuiUserSettingsManager();
		~GuiUserSettingsManager();

		// Load and save methods are called from the application side 
		// from Gui Panel Manager at startup or main menu panel by user request.
		void UserSettingsLoad(bool restoreDefaults);
		void UserSettingsSave();

	private:

		//std::filesystem::path m_userSettingsDataFilePath;

	};


} // namespace mnemosy::systems


#endif // !GUI_USER_SETTINGS_MANAGER_H
