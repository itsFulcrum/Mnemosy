#include "Include/Core/FileDirectories.h"

#include "Include/Core/Log.h"
//#include "Include/Core/JsonSettings.h"

#include <json.hpp>
#include <fstream>

namespace mnemosy::core
{
	FileDirectories::FileDirectories() {
		namespace fs = std::filesystem;


		m_mnemosyInternalResourcesDirectory = fs::directory_entry(R"(../Resources)");

		m_rootMaterialLibraryFolderName = "MnemosyMaterialLibrary";
		m_userLibraryDataFileName = "MnemosyMaterialLibraryData.mnsydata";


		m_mnemosyDefaultLibraryDirectory = fs::directory_entry(R"(C:/Users/Public/Documents/Mnemosy/MnemosyMaterialLibrary)");

		if (!m_mnemosyDefaultLibraryDirectory.exists()) {
			MNEMOSY_INFO("Default Mnemosy Directory does not yet exist. Creating directories {}", m_mnemosyDefaultLibraryDirectory.path().generic_string());
			fs::create_directories(m_mnemosyDefaultLibraryDirectory.path());
		}

		m_mnemosyLibraryDataFile = fs::directory_entry(R"(../Resources/Data/LibraryDirectory.mnsydata)");

		LoadUserLibraryDirectoryFromDataFile();
	}

	FileDirectories::~FileDirectories() 
	{ }


	const std::filesystem::path FileDirectories::GetResourcesPath() {

		if (m_mnemosyInternalResourcesDirectory.exists()) 
		{
			return m_mnemosyInternalResourcesDirectory.path();
		}
		else
		{
			MNEMOSY_ERROR("Resources Path does not exist.. Did you delete it?: {} ", m_mnemosyInternalResourcesDirectory.path().generic_string());
		}

		return std::filesystem::path();
	}

	const std::filesystem::path FileDirectories::GetMeshesPath() {

		std::filesystem::path meshesPath = m_mnemosyInternalResourcesDirectory.path() / std::filesystem::path("Meshes");
		return meshesPath;
	}

	const std::filesystem::path FileDirectories::GetPreviewMeshesPath() {

		return m_mnemosyInternalResourcesDirectory.path() / std::filesystem::path("Meshes/PreviewMeshes");
	}

	const std::filesystem::path FileDirectories::GetTexturesPath() {

		std::filesystem::path texturesPath = m_mnemosyInternalResourcesDirectory.path() / std::filesystem::path("Textures");
		return texturesPath;
	}

	const std::filesystem::path FileDirectories::GetDataPath() {

		std::filesystem::path dataPath = m_mnemosyInternalResourcesDirectory.path() / std::filesystem::path("Data");
		return dataPath;
	}

	const std::filesystem::path FileDirectories::GetCubemapsPath() {
		std::filesystem::path cubemapsPath = GetTexturesPath() / std::filesystem::path("Cubemaps");
		return cubemapsPath;
	}

	const std::filesystem::path FileDirectories::GetShadersPath() {

		std::filesystem::path shadersPath = m_mnemosyInternalResourcesDirectory.path() / std::filesystem::path("Shaders");
		return shadersPath;
	}

	const std::filesystem::path FileDirectories::GetDocumentationFilesPath() {
		return GetDataPath() / std::filesystem::path("DocumentationTextFiles");
	}

	const std::filesystem::path FileDirectories::GetUserLibDataFile() {

		namespace fs = std::filesystem;

		fs::path userLibDataFile = m_mnemosyUserLibraryDirectory.path().parent_path() / fs::path(m_userLibraryDataFileName);

		fs::directory_entry dirEntry;
		try {

			dirEntry = fs::directory_entry(userLibDataFile);
		}
		catch (fs::filesystem_error error) {
			MNEMOSY_ERROR("System error creating direcotry instance: \nError Message: {}", error.what());
		}


		if (!dirEntry.exists() || !dirEntry.is_regular_file() || dirEntry.is_directory()) {
			MNEMOSY_WARN("FileDirectories::GetUserLibDataFile: The filepath does not point to a valid file. {}", userLibDataFile.generic_string());
		}


		return userLibDataFile;
	}

	const std::filesystem::path FileDirectories::GetUserSettingsPath() {

		std::filesystem::path p = GetDataPath() / std::filesystem::path("UserSettings");

		std::filesystem::directory_entry dir = std::filesystem::directory_entry(p);

		if (!dir.exists()) {

			try {
				std::filesystem::create_directories(p);
			}
			catch (std::filesystem::filesystem_error err){
				
				MNEMOSY_CRITICAL("Faild to create directory for User Settings. \nMessage: {}", err.what());
			}
		}

		return p;
	}
	
	const std::filesystem::path FileDirectories::GetLibraryDirectoryPath() {
		
		return m_mnemosyUserLibraryDirectory.path();
	}

	void FileDirectories::SetNewUserLibraryDirectory(const std::filesystem::directory_entry& directoryPath, bool copyOldFiles, bool deleteOldFiles) {
		
		namespace fs = std::filesystem;

		// check if new path is a valid path
		if (!directoryPath.exists()) {
			MNEMOSY_ERROR("FileDirectories::SetNewUserLibraryDirectory: Directory entry does not exist: {}", directoryPath.path().generic_string());
			return;
		}
		if (!directoryPath.is_directory()) {
			MNEMOSY_ERROR("FileDirectories::SetNewUserLibraryDirectory: Directory entry is not a directory: {}", directoryPath.path().generic_string());
			return;
		}

		if (!fs::is_empty(directoryPath.path())) {

			MNEMOSY_ERROR("The folder you selected is not empty, please select a folder that is empty for the material library: path {}", directoryPath.path().generic_string());
			return;
		}

		fs::path newDirPath = directoryPath.path() / fs::path(m_rootMaterialLibraryFolderName);
		fs::directory_entry newLibraryDirectory = fs::directory_entry(newDirPath);


		if (copyOldFiles) {
			// copy all existing files to the new directory..
			try {


				fs::path dataFileFrom = GetUserLibDataFile();
				fs::path dataFileTo = directoryPath.path() / fs::path(m_userLibraryDataFileName);

				fs::copy(dataFileFrom, dataFileTo);

				fs::copy(m_mnemosyUserLibraryDirectory.path(), newDirPath, fs::copy_options::recursive);
			}
			catch (fs::filesystem_error error) {
				MNEMOSY_ERROR("FileDirectories::SetNewUserLibraryDirectory: System Error Copying directory: \nMessage: {}", error.what());
				return;
			}	
		}
		
		if (deleteOldFiles) {
			// delete contents of the old directory 
			try {

				fs::remove(GetUserLibDataFile());

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


	bool FileDirectories::LoadExistingMaterialLibrary(const std::filesystem::path& materialLibraryFolder, bool deleteOldFiles, bool saveAsNewLibraryDirectory) {
		// !! == before alling this function we should check if the filepath is valid


		namespace fs = std::filesystem;
				
		if (deleteOldFiles) {
			// delete contents of the old directory 
			try {
				// delete the data file
				fs::remove(GetUserLibDataFile());
				// delete the entire library folder
				fs::remove_all(m_mnemosyUserLibraryDirectory.path());
			}
			catch (fs::filesystem_error error) {
				MNEMOSY_ERROR("FileDirectories::SetNewUserLibraryDirectory: System Error Removing old directory: \nMessage: {}", error.what());
				return false;
			}
		}

		fs::directory_entry libraryFolderDir = fs::directory_entry(materialLibraryFolder);
		
		m_mnemosyUserLibraryDirectory = libraryFolderDir;

		if (saveAsNewLibraryDirectory) {
			SaveUserLibraryDirectoryToDataFile(libraryFolderDir);
		}

		return true;

	}

	bool FileDirectories::ContainsUserData() {

		return !std::filesystem::is_empty(m_mnemosyUserLibraryDirectory.path());
	}

	// private methods
	void FileDirectories::LoadUserLibraryDirectoryFromDataFile() {

		namespace fs = std::filesystem;

		bool success;
		flcrm::JsonSettings libDir;

		fs::path dataFilePath = m_mnemosyLibraryDataFile.path();


		libDir.FilePrettyPrintSet(true);

		libDir.FileOpen(success, dataFilePath, "Mnemosy Data File", "This Stores the path to the current library directory");
		if(!success){
			MNEMOSY_WARN("FileDirectories::LoadUserLibrary: Failed to open library directory data file. Message: {}", libDir.ErrorStringLastGet());
		}


		std::string libraryPathFromFile = libDir.ReadString(success,"libraryDirectory",m_mnemosyDefaultLibraryDirectory.path().generic_string(),true);

		libDir.FileClose(success,dataFilePath);


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



		return;


		// namespace fs = std::filesystem;

		// // check if data file exists 
		// if(!CheckLibraryDataFile()) {
		// 	//	if not, create file and set user path to default path
		// 	m_mnemosyUserLibraryDirectory = m_mnemosyDefaultLibraryDirectory;
		// 	// saveDataFile with default file as userFile but with bool flag that user has not set it custom
		// 	SaveUserLibraryDirectoryToDataFile(m_mnemosyDefaultLibraryDirectory);
		// 	return;
		// }
		
		// //data file exists
		
		// // read data file and extract path

		// std::fstream dataFileStream;
		// dataFileStream.open(m_mnemosyLibraryDataFile.path());

		// nlohmann::json readFile;
		// try {
		// 	readFile = nlohmann::json::parse(dataFileStream);
		// }
		// catch (nlohmann::json::parse_error err) {
		// 	MNEMOSY_ERROR("FileDirectories::LoadUserLibraryDirectoryFromDataFile: Error Parsing File. Message: {}", err.what());
		// 	return;
		// }
		// dataFileStream.close();

		// std::string libraryPathFromFile = readFile["Directories"]["MaterialLibraryDirectory"].get<std::string>();
		// readFile.clear();

		
		// // checking if path is valid
		// fs::directory_entry directoryEntryFromFile;
		// try {
		// 	directoryEntryFromFile = fs::directory_entry(libraryPathFromFile);
		// }
		// catch (fs::filesystem_error err) {
		// 	MNEMOSY_ERROR("FileDirectories::LoadUserLibraryDirectoryFromDataFile: System error initilizing directory {}\nError Message: {}", libraryPathFromFile,err.what());
		// 	SetDefaultLibraryPath();
		// 	return;
		// }

		// // final check if the path is valid
		// if (!directoryEntryFromFile.exists()) {
		// 	// if not create it
		// 	MNEMOSY_ERROR("FileDirectories::LoadUserLibraryDirectoryFromDataFile: Directory Does not exists..  did you delete it ? {} ", directoryEntryFromFile.path().generic_string());
		// 	SetDefaultLibraryPath();
		// 	return;
		// }

		// // when we reach here, the path read from file should exsist and be valid
		// m_mnemosyUserLibraryDirectory = fs::directory_entry(libraryPathFromFile);

	}

	void FileDirectories::SaveUserLibraryDirectoryToDataFile(const std::filesystem::directory_entry& libraryDirectoryPath) {
		

		// check if path is a valid path on disc.
		if (!libraryDirectoryPath.exists()) { 
			
			// if not create it
			MNEMOSY_INFO("FileDirectories::SaveUserLibraryDirectoryToDataFile: Directory Does not exists.. creating Directories {} ", libraryDirectoryPath.path().generic_string());
			std::filesystem::create_directories(libraryDirectoryPath.path());
		}
		else { // exists

			if (!libraryDirectoryPath.is_directory())
			{
				MNEMOSY_ERROR("FileDirectories::SaveUserLibraryDirectoryToDataFile: Path {} is not a directory.", libraryDirectoryPath.path().generic_string());
					return;
			}
		}
		
		MNEMOSY_INFO("Saving {} as library directory", libraryDirectoryPath.path().generic_string());

		m_mnemosyUserLibraryDirectory = libraryDirectoryPath;


		bool success;

		flcrm::JsonSettings libDirFile;

		std::filesystem::path dataFilePath = m_mnemosyLibraryDataFile.path();

		//std::filesystem::path libDirPath = libraryDirectoryPath.path();

		libDirFile.FilePrettyPrintSet(true);

		libDirFile.FileOpen(success, dataFilePath, "Mnemosy Data File", "This Stores the path to the current library directory");
		if(!success){
			MNEMOSY_WARN("FileDirectories::SaveUserLibrary: Failed to open library directory data file. Message: {}", libDirFile.ErrorStringLastGet());
		}

		libDirFile.WriteString(success,"libraryDirectory",libraryDirectoryPath.path().generic_string());

		libDirFile.FileClose(success,dataFilePath);

		return;


/*		// check if path is a valid path on disc.
		if (!libraryDirectoryPath.exists()) { 
			
			// if not create it
			MNEMOSY_INFO("FileDirectories::SaveUserLibraryDirectoryToDataFile: Directory Does not exists.. creating Directories {} ", libraryDirectoryPath.path().generic_string());
			std::filesystem::create_directories(libraryDirectoryPath.path());
		}
		else { // exists

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


		dataFileStream.close();*/
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
			MNEMOSY_ERROR("FileDirectories::CheckLibraryDataFile: File is not a regular file: {} \nCreating new file at that location", dataFilePath);
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
			std::filesystem::create_directories(m_mnemosyDefaultLibraryDirectory.path());
		}

		m_mnemosyUserLibraryDirectory = m_mnemosyDefaultLibraryDirectory;
		MNEMOSY_ERROR("Setting Library path to default Default path: {} ", m_mnemosyDefaultLibraryDirectory.path().generic_string());
	}


} // namespace mnemosy::core