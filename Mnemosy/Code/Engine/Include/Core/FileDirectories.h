#ifndef FILE_DIRECTORIES_H
#define FILE_DIRECTORIES_H

#include <filesystem>
#include <string>

namespace mnemosy::core
{

	struct LibraryCollection {
		std::filesystem::path folderPath;
		std::string name;
	};
	
	class FileDirectories
	{


	public:
		FileDirectories() = default;
		~FileDirectories() = default;

		void Init();
		void Shutdown();


		const std::filesystem::path& GetResourcesPath();
		const std::filesystem::path GetMeshesPath();
		const std::filesystem::path GetPreviewMeshesPath();
		const std::filesystem::path& GetDataPath();
		const std::filesystem::path GetUserSettingsPath();

		const std::filesystem::path GetTexturesPath();
		const std::filesystem::path GetCubemapsPath();
		const std::filesystem::path GetShadersPath();
		const std::filesystem::path GetDocumentationFilesPath();
	private:

		// reducing memory footprint a bit

		std::filesystem::path m_internalResourcesFolderPath;
		//std::filesystem::path m_internalMeshesFolderPath;
		//std::filesystem::path m_internalPreviewMeshesFolderPath;
		//std::filesystem::path m_internalTexturesFolderPath;
		std::filesystem::path m_internalDataFolderPath;
		//std::filesystem::path m_internalCubemapsFolderPath;
		//std::filesystem::path m_internalShadersFolderPath;
		//std::filesystem::path m_internalUserSettingsFolderPath;
		//std::filesystem::path m_internalDocumentationFolderPath;
	};

} // !mnemosy::core

#endif // !FILE_DIRECTORIES_H
