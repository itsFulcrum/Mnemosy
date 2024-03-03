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

		fs::path GetResourcesPath();
		fs::path GetMeshesPath();
		fs::path GetTexturesPath();
		fs::path GetDataPath();
		fs::path GetCubemapsPath();
		fs::path GetShadersPath();


	private:
		//std::filesystem::path m_mnemosyInternalResourcesDirectory;

		//std::filesystem::path m_userLibraryFileDirectory;

		fs::directory_entry m_mnemosyInternalResourcesDirectory;
	};

} // !mnemosy::core

#endif // !FILE_DIRECTORIES_H
