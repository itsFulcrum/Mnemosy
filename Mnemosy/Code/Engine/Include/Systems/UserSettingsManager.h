#ifndef USER_SETTINGS_MANAGER_H
#define USER_SETTINGS_MANAGER_H


#include <filesystem>


namespace mnemosy::systems {

	class UserSettingsManager {

	public:
		UserSettingsManager();
		~UserSettingsManager();

		void LoadUserSettings(bool useDefaultFile);
		void SaveToFile();

	private:
		bool CheckDataFile(std::filesystem::directory_entry& fileDirectoryEntry);

		std::filesystem::path m_userSettingsDataFilePath;

	};


} // namespace mnemosy::systems


#endif // !USER_SETTINGS_MANAGER_H
