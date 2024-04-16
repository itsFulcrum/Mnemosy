#include "Include/Core/FileDirectories.h"

#include "Include/Core/Log.h"

#include <nlohmann/json.hpp>
#include <fstream>

namespace mnemosy::core
{
	FileDirectories::FileDirectories() {

		m_mnemosyInternalResourcesDirectory = fs::directory_entry(R"(../Resources)");
		m_rootMaterialLibraryFolderName = "MnemosyMaterialLibrary";
		m_tempExportTextureFolderName = "__Temp_Mnemosy_Export__";

		m_mnemosyDefaultLibraryDirectory = fs::directory_entry(R"(C:/Users/Public/Documents/Mnemosy/MnemosyMaterialLibrary)");
		if (!m_mnemosyDefaultLibraryDirectory.exists()) {
			MNEMOSY_WARN("Default Mnemosy Directory does not yet exist. Creating directories {}", m_mnemosyDefaultLibraryDirectory.path().generic_string());
			fs::create_directories(m_mnemosyDefaultLibraryDirectory.path());
		}

		m_mnemosyLibraryDataFile = fs::directory_entry(R"(../Resources/Data/LibraryDirectory.mnsydata)");

		LoadUserLibraryDirectoryFromDataFile();
		
		DeleteTempFolder();
	}

	FileDirectories::~FileDirectories() {

		DeleteTempFolder();
	}

	const fs::path FileDirectories::GetResourcesPath() {

		if (m_mnemosyInternalResourcesDirectory.exists()) 
		{
			return m_mnemosyInternalResourcesDirectory.path();
		}
		else
		{
			MNEMOSY_ERROR("Resources Path does not exist.. Did you delete it?: {} ", m_mnemosyInternalResourcesDirectory.path().generic_string());
		}

		return std::filesystem::path("");
	}

	const fs::path FileDirectories::GetMeshesPath() {
		fs::path meshesPath = m_mnemosyInternalResourcesDirectory.path() / fs::path("Meshes");
		return meshesPath;
	}

	const fs::path FileDirectories::GetPreviewMeshesPath()
	{
		return m_mnemosyInternalResourcesDirectory.path() / fs::path("Meshes/PreviewMeshes");
	}

	const fs::path FileDirectories::GetTexturesPath() {
		fs::path texturesPath = m_mnemosyInternalResourcesDirectory.path() / fs::path("Textures");
		return texturesPath;
	}

	const fs::path FileDirectories::GetDataPath() {
		fs::path dataPath = m_mnemosyInternalResourcesDirectory.path() / fs::path("Data");
		return dataPath;
	}

	const fs::path FileDirectories::GetCubemapsPath() {
		fs::path cubemapsPath = GetTexturesPath() / fs::path("Cubemaps");
		return cubemapsPath;
	}

	const fs::path FileDirectories::GetShadersPath() {

		fs::path shadersPath = m_mnemosyInternalResourcesDirectory.path() / fs::path("Shaders");
		return shadersPath;
	}

	const fs::path FileDirectories::GetTempExportFolderPath()
	{
		fs::path tempFolderPath = GetLibraryDirectoryPath() / fs::path(m_tempExportTextureFolderName);
		
		if (!TempFolderExist()) {

			if (CreateTempFolder()) {				
				return tempFolderPath;
			}
			else {
				MNEMOSY_ERROR("FileDirectories::ErrorCreatingTempFolder:");
				return fs::path();
			}			
		}

		return tempFolderPath;
	}
	
	const fs::path FileDirectories::GetLibraryDirectoryPath() {
		
		return m_mnemosyUserLibraryDirectory.path();
	}

	void FileDirectories::SetNewUserLibraryDirectory(const fs::directory_entry& directoryPath, bool copyOldFiles, bool deleteOldFiles) {
		
		// check if new path is a valid path
		if (!directoryPath.exists()) {
			MNEMOSY_ERROR("FileDirectories::SetNewUserLibraryDirectory: Directory entry does not exist: {}", directoryPath.path().generic_string());
			return;
		}
		if (!directoryPath.is_directory()) {
			MNEMOSY_ERROR("FileDirectories::SetNewUserLibraryDirectory: Directory entry is not a directory: {}", directoryPath.path().generic_string());
			return;
		}

		fs::path newDirPath = directoryPath.path() / fs::path(m_rootMaterialLibraryFolderName);
		fs::directory_entry newLibraryDirectory = fs::directory_entry(newDirPath);


		if (copyOldFiles) {
			// copy all existing files to the new directory..
			try {
				fs::copy(m_mnemosyUserLibraryDirectory.path(), newDirPath, fs::copy_options::recursive);
			}
			catch (fs::filesystem_error error) {
				MNEMOSY_ERROR("FileDirectories::SetNewUserLibraryDirectory: System Error Copying directory: \nMessage: {}", error.what());
				return;
			}
			
			
		}
		else {
			


			
		}
		
		if (deleteOldFiles) {
			// delete contents of the old directory 
			try {
				fs::remove_all(m_mnemosyUserLibraryDirectory.path());
			}
			catch (fs::filesystem_error error) {
				MNEMOSY_ERROR("FileDirectories::SetNewUserLibraryDirectory: System Error Removing old directory: \nMessage: {}", error.what());
				return;
			}
		}
		//MNEMOSY_WARN("Setting library Directories to Path: {}", directoryPath.path().generic_string())



		SaveUserLibraryDirectoryToDataFile(newLibraryDirectory);

	}

	bool FileDirectories::ContainsUserData() {

		return !fs::is_empty(m_mnemosyUserLibraryDirectory.path());
	}

	// private methods

	void FileDirectories::LoadUserLibraryDirectoryFromDataFile() {

		// check if data file exists 
		if(!CheckLibraryDataFile()) {
			//	if not, create file and set user path to default path
			m_mnemosyUserLibraryDirectory = m_mnemosyDefaultLibraryDirectory;
			// saveDataFile with default file as userFile but with bool flag that user has not set it custom
			SaveUserLibraryDirectoryToDataFile(m_mnemosyDefaultLibraryDirectory);
			return;
		}
		
		//data file exists
		
		// read data file and extract path;
		std::string pathToDataFile = m_mnemosyLibraryDataFile.path().generic_string();

		std::string libraryPathFromFile = "";

		std::fstream dataFileStream;
		dataFileStream.open(pathToDataFile);



		nlohmann::json readFile;
		try {
			readFile = nlohmann::json::parse(dataFileStream);
		}
		catch (nlohmann::json::parse_error err) {
			MNEMOSY_ERROR("FileDirectories::LoadUserLibraryDirectoryFromDataFile: Error Parsing File. Message: {}", err.what());
			return;
		}

		libraryPathFromFile = readFile["Directories"]["MaterialLibraryDirectory"].get<std::string>();
		dataFileStream.close();

		readFile.clear();
		
		// checking if path is valid
		fs::directory_entry directoryEntryFromFile;
		try {
			directoryEntryFromFile = fs::directory_entry(libraryPathFromFile);
		}
		catch (fs::filesystem_error err) {
			MNEMOSY_ERROR("FileDirectories::LoadUserLibraryDirectoryFromDataFile: System error initilizing directory {}\nError Message: {}", libraryPathFromFile,err.what());
			SetDefaultLibraryPath();
			return;
		}

		// final check if the path is valid
		if (!directoryEntryFromFile.exists()) {

			// if not create it
			MNEMOSY_ERROR("FileDirectories::LoadUserLibraryDirectoryFromDataFile: Directory Does not exists..  did you delete it ? {} ", directoryEntryFromFile.path().generic_string());
			SetDefaultLibraryPath();
			return;
		}

		// when we reach here, the path read from file should exsist and be valid
		m_mnemosyUserLibraryDirectory = fs::directory_entry(libraryPathFromFile);

	}

	void FileDirectories::SaveUserLibraryDirectoryToDataFile(const fs::directory_entry& libraryDirectoryPath) {
		// check if path is a valid path on disc.
		if (!libraryDirectoryPath.exists()) { 
			
			// if not create it
			MNEMOSY_INFO("FileDirectories::SaveUserLibraryDirectoryToDataFile: Directory Does not exists.. creating Directories {} ", libraryDirectoryPath.path().generic_string());
			fs::create_directories(libraryDirectoryPath.path());
		}
		else { // exists

			//MNEMOSY_INFO("exists..");
			if (!libraryDirectoryPath.is_directory())
			{
				MNEMOSY_ERROR("FileDirectories::SaveUserLibraryDirectoryToDataFile: Path {} is not a directory.", libraryDirectoryPath.path().generic_string());
					return;
			}
		}
		
		MNEMOSY_INFO("Saving {} as library directory", libraryDirectoryPath.path().generic_string());

		m_mnemosyUserLibraryDirectory = libraryDirectoryPath;

		CheckLibraryDataFile();
		std::string pathToDataFile = m_mnemosyLibraryDataFile.path().generic_string();
		std::ofstream dataFileStream;
		// clear file first // idk this seems kinda stupid and dangerous but also it gets mees up when just overwriting it // maybe do backup copy first?
		dataFileStream.open(pathToDataFile);
		dataFileStream << "";
		dataFileStream.close();

		// start Saving
		dataFileStream.open(pathToDataFile);


		nlohmann::json LibraryDataFileJson; // top level json object
		LibraryDataFileJson["1_Mnemosy_Data_File"] = "LibraryDirectories";

		std::string libraryDirectoryString = libraryDirectoryPath.path().generic_string();
		nlohmann::json DirectoriesJson;
		DirectoriesJson["MaterialLibraryDirectory"] = libraryDirectoryString;

		LibraryDataFileJson["Directories"] = DirectoriesJson;

		
		if (prettyPrintDataFile)
			dataFileStream << LibraryDataFileJson.dump(2);
		else
			dataFileStream << LibraryDataFileJson.dump(-1);


		dataFileStream.close();
	}

	bool FileDirectories::CheckLibraryDataFile() {
		
		if (!m_mnemosyLibraryDataFile.exists()) {

			std::string dataFilePath = m_mnemosyLibraryDataFile.path().generic_string();
			MNEMOSY_WARN("FileDirectories::CheckLibraryDataFile: File did Not Exist: {} \nCreating new file at that location", dataFilePath);
			std::ofstream file;
			file.open(dataFilePath);
			file << "";
			file.close();
			return false;
		}
		if (!m_mnemosyLibraryDataFile.is_regular_file()) {

			std::string dataFilePath = m_mnemosyLibraryDataFile.path().generic_string();
			MNEMOSY_ERROR("SkyboxAssetRegistry::CheckDataFile: File is not a regular file: {} \nCreating new file at that location", dataFilePath);
			// maybe need to delete unregular file first idk should never happen anyhow
			std::ofstream file;
			file.open(dataFilePath);
			file << "";
			file.close();
			return false;
		}

		return true;
	}

	void FileDirectories::SetDefaultLibraryPath() {
		// checking if default path  exists if not creating it
		if (!m_mnemosyDefaultLibraryDirectory.exists()) {

			MNEMOSY_WARN("FileDirectories::SetDefaultLibraryPath: Default Mnemosy Directory does not exist yet. Creating directories at {}", m_mnemosyDefaultLibraryDirectory.path().generic_string());
			fs::create_directories(m_mnemosyDefaultLibraryDirectory.path());
		}

		m_mnemosyUserLibraryDirectory = m_mnemosyDefaultLibraryDirectory;
		MNEMOSY_ERROR("Setting Library path to default Default path: {} ", m_mnemosyDefaultLibraryDirectory.path().generic_string());
	}

	bool FileDirectories::CreateTempFolder() {

		if (!TempFolderExist()) {
			fs::path tempFolderPath = GetLibraryDirectoryPath() / fs::path(m_tempExportTextureFolderName);
			try {
				fs::create_directories(tempFolderPath);
			}
			catch (fs::filesystem_error err) {
				MNEMOSY_ERROR("FileDirectories::CreateTempFolder: System error creating directory {} \nError Message: {}", tempFolderPath.generic_string(), err.what());
				return false;
			}
		}

		return true;
	}

	void FileDirectories::DeleteTempFolder() {

		fs::path tempFolderPath = GetLibraryDirectoryPath() / fs::path(m_tempExportTextureFolderName);		
		
		if (TempFolderExist()) {
			fs::remove_all(tempFolderPath);
		}

	}

	bool FileDirectories::TempFolderExist(){

		fs::path tempFolderPath = GetLibraryDirectoryPath() / fs::path(m_tempExportTextureFolderName);
		fs::directory_entry tempFolderDir = fs::directory_entry(tempFolderPath);

		if (tempFolderDir.exists()) {
			if (tempFolderDir.is_directory()) {

				return true;
			}
		}

		return false;
	}


} // !mnemosy::core