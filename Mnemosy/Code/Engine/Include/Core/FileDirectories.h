#ifndef FILE_DIRECTORIES_H
#define FILE_DIRECTORIES_H

#include <filesystem>
#include <string>

namespace mnemosy::core
{
	
	class FileDirectories
	{


	public:
		FileDirectories() = default;
		~FileDirectories() = default;

		void Init();


		const std::filesystem::path GetResourcesPath();
		const std::filesystem::path GetMeshesPath();
		const std::filesystem::path GetPreviewMeshesPath();
		const std::filesystem::path GetTexturesPath();
		const std::filesystem::path GetDataPath();
		const std::filesystem::path GetCubemapsPath();
		const std::filesystem::path GetShadersPath();
		const std::filesystem::path GetDocumentationFilesPath();
		const std::filesystem::path GetUserLibDataFile();
		const std::filesystem::path GetUserSettingsPath();

		const std::filesystem::path GetLibraryDirectoryPath();

		void SetNewUserLibraryDirectory(const std::filesystem::directory_entry& directoryPath,bool copyOldFiles,bool deleteOldFiles);

		bool LoadExistingMaterialLibrary(const std::filesystem::path& folderPath, bool deleteOldFiles, bool saveAsNewLibraryDirectory);

		bool ContainsUserData();


		

	private:
		bool prettyPrintDataFile = false;
		std::string m_rootMaterialLibraryFolderName;// = "MnemosyMaterialLibrary";
		std::string m_userLibraryDataFileName;
		void LoadUserLibraryDirectoryFromDataFile();
		void SaveUserLibraryDirectoryToDataFile(const std::filesystem::directory_entry& libraryDirectoryPath);
		bool CheckLibraryDataFile();
		void SetDefaultLibraryPath();


		std::filesystem::directory_entry m_mnemosyInternalResourcesDirectory;
		std::filesystem::directory_entry m_mnemosyUserLibraryDirectory;
		std::filesystem::directory_entry m_mnemosyDefaultLibraryDirectory;
		std::filesystem::directory_entry m_mnemosyLibraryDataFile;
	};

} // !mnemosy::core

#endif // !FILE_DIRECTORIES_H
