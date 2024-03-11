#ifndef FILE_DIRECTORIES_H
#define FILE_DIRECTORIES_H

#include <filesystem>
#include <string>


namespace fs = std::filesystem;

namespace mnemosy::core
{
	class FileDirectories
	{
	public:
		FileDirectories();
		~FileDirectories();

		const fs::path GetResourcesPath();
		const fs::path GetMeshesPath();
		const fs::path GetTexturesPath();
		const fs::path GetDataPath();
		const fs::path GetCubemapsPath();
		const fs::path GetShadersPath();

		const fs::path GetLibraryDirectoryPath();
		void SetUserLibraryDirectory(const fs::directory_entry& directoryPath);

	private:
		bool prettyPrintDataFile = true;


		void LoadUserLibraryDirectoryFromDataFile();
		void SaveUserLibraryDirectoryToDataFile(const fs::directory_entry& libraryDirectoryPath);
		bool CheckLibraryDataFile();
		void SetDefaultLibraryPath();
		//std::filesystem::path m_mnemosyInternalResourcesDirectory;

		//std::filesystem::path m_userLibraryFileDirectory;

		fs::directory_entry m_mnemosyInternalResourcesDirectory;
		fs::directory_entry m_mnemosyUserLibraryDirectory;
		fs::directory_entry m_mnemosyDefaultLibraryDirectory;
		fs::directory_entry m_mnemosyLibraryDataFile;
	};

} // !mnemosy::core

#endif // !FILE_DIRECTORIES_H
