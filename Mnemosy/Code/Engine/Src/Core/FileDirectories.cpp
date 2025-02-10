#include "Include/Core/FileDirectories.h"

#include "Include/MnemosyConfig.h"
#include "Include/Core/Log.h"


namespace mnemosy::core {

	void FileDirectories::Init() {

		namespace fs = std::filesystem;

		m_internalResourcesFolderPath = fs::path(R"(../Resources)");

		//m_internalMeshesFolderPath			= GetResourcesPath() / fs::path("Meshes");
		//m_internalPreviewMeshesFolderPath	= GetMeshesPath() / fs::path("PreviewMeshes");	
		//m_internalTexturesFolderPath		= GetResourcesPath() / fs::path("Textures");
		//m_internalCubemapsFolderPath		= GetTexturesPath() / fs::path("Cubemaps");
		m_internalDataFolderPath			= GetResourcesPath() / fs::path("Data");
		//m_internalUserSettingsFolderPath	= GetDataPath() / fs::path("UserSettings");
		//m_internalShadersFolderPath			= GetResourcesPath() / fs::path("Shaders");
		//m_internalDocumentationFolderPath	= GetDataPath() / fs::path("DocumentationTextFiles");

	}

	void FileDirectories::Shutdown()
	{
	}


	const std::filesystem::path& FileDirectories::GetResourcesPath() {
		namespace fs = std::filesystem;

		if (fs::exists(m_internalResourcesFolderPath)) {

			return m_internalResourcesFolderPath;
		}
		else {

			MNEMOSY_ERROR("Resources Path does not exist.. Did you delete it?  Creating New {} ", m_internalResourcesFolderPath.generic_string());

			fs::create_directories(m_internalResourcesFolderPath);
			return m_internalResourcesFolderPath;
		}

		return m_internalResourcesFolderPath;
	}

	const std::filesystem::path FileDirectories::GetMeshesPath() {

		namespace fs = std::filesystem;

		fs::path meshesPath = GetResourcesPath() / fs::path("Meshes");

		if (fs::exists(meshesPath)) {

			return meshesPath;
		}
		else {

			MNEMOSY_ERROR("Filepath does not exist.. Did you delete it?  Creating New {} ", meshesPath.generic_string());

			fs::create_directories(meshesPath);
			return meshesPath;
		}

		return meshesPath;
	}

	const std::filesystem::path FileDirectories::GetPreviewMeshesPath() {

		namespace fs = std::filesystem;

		fs::path m_internalPreviewMeshesFolderPath = GetMeshesPath() / fs::path("PreviewMeshes");

		if (fs::exists(m_internalPreviewMeshesFolderPath)) {

			return m_internalPreviewMeshesFolderPath;
		}
		else {

			MNEMOSY_ERROR("Filepath does not exist.. Did you delete it?  Creating New {} ", m_internalPreviewMeshesFolderPath.generic_string());

			fs::create_directories(m_internalPreviewMeshesFolderPath);
			return m_internalPreviewMeshesFolderPath;
		}

		return m_internalPreviewMeshesFolderPath;
	}
	
	const std::filesystem::path& FileDirectories::GetDataPath() {

		namespace fs = std::filesystem;

		if (fs::exists(m_internalDataFolderPath)) {

			return m_internalDataFolderPath;
		}
		else {

			MNEMOSY_ERROR("Filepath does not exist.. Did you delete it?  Creating New {} ", m_internalDataFolderPath.generic_string());

			fs::create_directories(m_internalDataFolderPath);
			return m_internalDataFolderPath;
		}

		return m_internalDataFolderPath;
	}
	
	const std::filesystem::path FileDirectories::GetUserSettingsPath() {

		namespace fs = std::filesystem;

		fs::path m_internalUserSettingsFolderPath = GetDataPath() / fs::path("UserSettings");

		if (fs::exists(m_internalUserSettingsFolderPath)) {

			return m_internalUserSettingsFolderPath;
		}
		else {

			MNEMOSY_ERROR("Filepath does not exist.. Did you delete it?  Creating New {} ", m_internalUserSettingsFolderPath.generic_string());

			fs::create_directories(m_internalUserSettingsFolderPath);
			return m_internalUserSettingsFolderPath;
		}

		return m_internalUserSettingsFolderPath;
	}

	const std::filesystem::path FileDirectories::GetFontsPath()
	{
		namespace fs = std::filesystem;


		fs::path fontsPath = GetResourcesPath() / fs::path("Fonts");

		if (fs::exists(fontsPath)) {

			return fontsPath;
		}
		else {

			MNEMOSY_ERROR("Filepath does not exist.. Did you delete it?  Creating New {} ", fontsPath.generic_string());

			fs::create_directories(fontsPath);
			return fontsPath;
		}


		return fontsPath;
	}
	
	const std::filesystem::path FileDirectories::GetTexturesPath() {

		namespace fs = std::filesystem;

		fs::path m_internalTexturesFolderPath = GetResourcesPath() / fs::path("Textures");

		if (fs::exists(m_internalTexturesFolderPath)) {

			return m_internalTexturesFolderPath;
		}
		else {

			MNEMOSY_ERROR("Filepath does not exist.. Did you delete it?  Creating New {} ", m_internalTexturesFolderPath.generic_string());

			fs::create_directories(m_internalTexturesFolderPath);
			return m_internalTexturesFolderPath;
		}

		return m_internalTexturesFolderPath;
	}

	const std::filesystem::path FileDirectories::GetCubemapsPath() {

		namespace fs = std::filesystem;

		fs::path m_internalCubemapsFolderPath = GetTexturesPath() / fs::path("Cubemaps");

		if (fs::exists(m_internalCubemapsFolderPath)) {

			return m_internalCubemapsFolderPath;
		}
		else {

			MNEMOSY_ERROR("Filepath does not exist.. Did you delete it?  Creating New {} ", m_internalCubemapsFolderPath.generic_string());

			fs::create_directories(m_internalCubemapsFolderPath);
			return m_internalCubemapsFolderPath;
		}

		return m_internalCubemapsFolderPath;
	}

	const std::filesystem::path FileDirectories::GetShadersPath() {
		namespace fs = std::filesystem;

		fs::path m_internalShadersFolderPath = GetResourcesPath() / fs::path("Shaders");

		if (fs::exists(m_internalShadersFolderPath)) {

			return m_internalShadersFolderPath;
		}
		else {

			MNEMOSY_ERROR("Filepath does not exist.. Did you delete it?  Creating New {} ", m_internalShadersFolderPath.generic_string());

			fs::create_directories(m_internalShadersFolderPath);
			return m_internalShadersFolderPath;
		}

		return m_internalShadersFolderPath;
	}

	const std::filesystem::path FileDirectories::GetDocumentationFilesPath() {
		namespace fs = std::filesystem;

		fs::path m_internalDocumentationFolderPath = GetDataPath() / fs::path("DocumentationTextFiles");

		if (fs::exists(m_internalDocumentationFolderPath)) {

			return m_internalDocumentationFolderPath;
		}
		else {

			MNEMOSY_ERROR("Filepath does not exist.. Did you delete it?  Creating New {} ", m_internalDocumentationFolderPath.generic_string());

			fs::create_directories(m_internalDocumentationFolderPath);
			return m_internalDocumentationFolderPath;
		}

		return m_internalDocumentationFolderPath;
	}


} // namespace mnemosy::core