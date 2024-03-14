#include "Include/Systems/MaterialLibraryRegistry.h"


#include "Include/Core/Log.h"
#include "Include/MnemosyEngine.h"
#include "Include/Core/FileDirectories.h"
#include "Include/Systems/FolderTreeNode.h"

#include <fstream>

namespace mnemosy::systems
{
	// == public methods
 
	MaterialLibraryRegistry::MaterialLibraryRegistry() 
		: m_fileDirectories{ MnemosyEngine::GetInstance().GetFileDirectories()}
	{

		m_runtimeIDCounter = 0;
		//mnemosy::core::FileDirectories& fd = MnemosyEngine::GetInstance().GetFileDirectories();
		fs::path pathToUserDirectoriesDataFile = m_fileDirectories.GetDataPath() / fs::path("UserLibraryDirectories.mnsydata");
		m_userDirectoriesDataFile = fs::directory_entry(pathToUserDirectoriesDataFile); 
		
		m_rootNodeName = "Root";

		LoadUserDirectoriesFromFile();

	}

	MaterialLibraryRegistry::~MaterialLibraryRegistry() {
		
		SaveUserDirectoriesData();
		// recursivly cleanup folderTree heap memory
		RecursivCleanFolderTreeMemory(m_rootFolderNode);
	}

	
	void MaterialLibraryRegistry::RenameDirectory(FolderNode* node, std::string oldPathFromRoot) {
		
		//mnemosy::core::FileDirectories& fd = MnemosyEngine::GetInstance().GetFileDirectories();
		fs::path libraryDir = m_fileDirectories.GetLibraryDirectoryPath();

		fs::path oldPath = libraryDir / fs::path(oldPathFromRoot);
		fs::path newPath = libraryDir / fs::path(node->pathFromRoot);

		try {
			fs::rename(oldPath, newPath);
		}
		catch (fs::filesystem_error error) {
			MNEMOSY_ERROR("MaterialLibraryRegistry::RenameDirectory: System error renaming directory: {} \nError message: {}", oldPath.generic_string(), newPath.generic_string(), error.what());
		}
		
		RecursivUpadtePathFromRoot(node);
	}

	void MaterialLibraryRegistry::MoveDirectory(FolderNode* dragSource, FolderNode* dragTarget) {

		fs::path libraryDir = MnemosyEngine::GetInstance().GetFileDirectories().GetLibraryDirectoryPath();
		fs::path fromPath = libraryDir / fs::path(dragSource->pathFromRoot);
		fs::path toPath = libraryDir / fs::path(dragTarget->pathFromRoot);

		// Copying and then removing works rn but is not very elegant. fs::rename() does not work and throws acces denied error
			//fs::rename(fromPath, toPath / fromPath.filename());

		try { // copy directory to new location
			fs::copy(fromPath, toPath / fromPath.filename(), fs::copy_options::recursive);
		}
		catch (fs::filesystem_error error) {
			MNEMOSY_ERROR("MaterialLibraryRegistry::MoveDirectory: System Error Copying directory: \nMessage: {}", error.what());
			return;
		}
		try { // remove old directory
			fs::remove_all(fromPath);
		}
		catch (fs::filesystem_error error) {
			MNEMOSY_ERROR("MaterialLibraryRegistry::MoveDirectory: System Error Removing old directory: \nMessage: {}", error.what());
			return;
		}
		
		// Updating Internal Data
		// removing from old parent
		for (int i = 0; i < dragSource->parent->subNodes.size(); i++) {
			if (dragSource->parent->subNodes[i]->name == dragSource->name) {
				dragSource->parent->subNodes.erase(dragSource->parent->subNodes.begin() + i);
			}
		}
		dragSource->parent = dragTarget; // set new parent
		dragTarget->subNodes.push_back(dragSource); // enlist into target subNodes
		// update path from root
		if (dragSource->parent->name == m_rootNodeName) {
			dragSource->pathFromRoot = dragSource->name; 
		}
		else {
			dragSource->pathFromRoot = dragSource->parent->pathFromRoot + "/" + dragSource->name; 
		}

		// should prob save to data
		SaveUserDirectoriesData();
	}

	void MaterialLibraryRegistry::DeleteFolderHierarchy(FolderNode* node) {

		// never delete root node
		if (node->parent == nullptr || node->name == m_rootNodeName) {
			MNEMOSY_WARN("You can't delete the root directory");
			return;
		}
		
		{ // delete directories from disk			
			fs::path libraryDir = MnemosyEngine::GetInstance().GetFileDirectories().GetLibraryDirectoryPath();
			fs::path directoryPathToDelete = libraryDir / fs::path(node->pathFromRoot);

			try {
				// this call removes all files and directories underneith permanently without moving it to trashbin
				fs::remove_all(directoryPathToDelete);
			}
			catch (fs::filesystem_error error) {
				MNEMOSY_ERROR("MaterialLibraryRegistry::DeleteFolderHierarchy: System error deleting directory: {} \nError Message: {} ", directoryPathToDelete.generic_string(), error.what());
			}
		}

		// remove from node from parent subnodes list
		for (int i = 0; i < node->parent->subNodes.size(); i++) {
			if (node->parent->subNodes[i]->name == node->name) {
				node->parent->subNodes.erase(node->parent->subNodes.begin() + i);
			}
		}

		// Free memory for all subsequent nodes recursivly
		RecursivCleanFolderTreeMemory(node);
	}


	FolderNode* MaterialLibraryRegistry::CreateFolderNode(FolderNode* parentNode, std::string name) {
		FolderNode* node = new FolderNode();
		node->name = name;
		node->parent = parentNode;

		std::string pathFromRoot = "";
		if (parentNode != nullptr) { // if this not root node
			if (parentNode->name == m_rootNodeName) { // if parent is root
				pathFromRoot = node->name;
			}
			else {
				pathFromRoot = parentNode->pathFromRoot + "/" + node->name;
			}
		}
		node->pathFromRoot = pathFromRoot;

		node->runtime_ID = m_runtimeIDCounter;
		m_runtimeIDCounter++;
		// create system folder 
		CreateDirectoryForNode(node);

		return node;
	}

	FolderNode* MaterialLibraryRegistry::GetRootFolder() {

		return m_rootFolderNode;
	}

	FolderNode* MaterialLibraryRegistry::RecursivGetNodeByRuntimeID(FolderNode* node, unsigned int id) {

		if (node == nullptr) {
			return nullptr;
		}

		if (node->runtime_ID == id) {
			return node;
		}

		FolderNode* foundNode = nullptr;
		for (FolderNode* child : node->subNodes) {

			foundNode = RecursivGetNodeByRuntimeID(child, id);
			if (foundNode != nullptr) {
				break; // Found the node, no need to search further
			}
		}

		return foundNode;
	}

	void MaterialLibraryRegistry::CreateNewMaterial(FolderNode* node, std::string name) {

		// check if name exists already and add suffix number if needed
		int suffix = 1;
		std::string finalName = name;
		while (node->SubMaterialExistsAlready(finalName)) {
			finalName = name + "_" + std::to_string(suffix);
			suffix++;
		}

		fs::path libraryDir = m_fileDirectories.GetLibraryDirectoryPath();
		// create directory for material
		fs::path materialDirectory = libraryDir / fs::path(node->pathFromRoot) / fs::path(finalName);
		MNEMOSY_TRACE("Add Material: new material directory: {}", materialDirectory.generic_string());

		try {
			fs::create_directory(materialDirectory);
		} catch (fs::filesystem_error error) {
			MNEMOSY_ERROR("MaterialLibraryRegistry::AddMaterial: System error creating directory. \nError message: {}", error.what());
			return;
		}
		// Render basic thumbnail image
		// 
		// create default material data file;
		// TODO:
		// write actual data into file

		
		CreateNewMaterialDataFile(materialDirectory,finalName);



		// create material instance and make it visible in the viewport and material editor

		// adding entry to list of directory node;
		node->subMaterialNames.push_back(finalName);

	}

	void MaterialLibraryRegistry::ChangeMaterialName(systems::FolderNode* node, std::string& materialName, std::string& newName, int positionInVector) {

		std::string oldName = materialName;
		if (materialName == newName)
			return;
		
		// check if name exists already and add suffix number if needed
		int suffix = 1;
		std::string finalName = newName;
		while (node->SubMaterialExistsAlready(finalName)) {
			finalName = newName + "_" + std::to_string(suffix);
			suffix++;
		}



		fs::path libraryDir = m_fileDirectories.GetLibraryDirectoryPath();

		// change name of thumbnail img

		// change names of all accosiated textures

		// change name of data file. TODO: and adjust all data inside
		{
			fs::path oldDataFilePath = libraryDir / fs::path(node->pathFromRoot) / fs::path(oldName) / fs::path(oldName + ".mnsydata");
			fs::path newDataFilePath = libraryDir / fs::path(node->pathFromRoot) / fs::path(oldName) / fs::path(finalName + ".mnsydata");
			
			std::fstream dataFileStream;
			dataFileStream.open(oldDataFilePath.generic_string());
			
			json readFile;
			try {
				readFile = json::parse(dataFileStream);
			}
			catch (json::parse_error err) {
				MNEMOSY_ERROR("MaterialLibraryRegistry::ChangeMaterialName: Error parsing json file. Message: {}", err.what());
				return;
			}
			dataFileStream.close();

			readFile["name"] = finalName;
			std::string thumbnailPath = finalName + "_thumbnail.ktx2";
			readFile["thumbnailPath"] = thumbnailPath;

			std::ofstream outFileStream;
			outFileStream.open(oldDataFilePath.generic_string());
			if (prettyPrintDataFile)
				outFileStream << readFile.dump(4);
			else
				outFileStream << readFile.dump(-1);
			outFileStream.close();

			try {
				fs::rename(oldDataFilePath, newDataFilePath);
			}
			catch (fs::filesystem_error error) {
				MNEMOSY_ERROR("MaterialLibraryRegistry::ChangeMaterialName: System error renaming dataFile. \nError message: {}", error.what());
				return;
			}
		}

		// change name of material folder.
		{
			fs::path oldPath = libraryDir / fs::path(node->pathFromRoot) / fs::path(oldName);
			fs::path newPath = libraryDir / fs::path(node->pathFromRoot) / fs::path(finalName);

			try {
				fs::rename(oldPath, newPath);
			}
			catch (fs::filesystem_error error) {
				MNEMOSY_ERROR("MaterialLibraryRegistry::ChangeMaterialName: System error renaming directory. \nError message: {}", error.what());
			}
		}


		// change name in vector of names..
		node->subMaterialNames[positionInVector] = finalName;
		// save library data file;
	}

	void MaterialLibraryRegistry::DeleteMaterial(FolderNode* node, std::string& materialName, int positionInVector) {

		fs::path libraryDir = m_fileDirectories.GetLibraryDirectoryPath();
		// delete files

		fs::path pathToMaterialDirectory = libraryDir / fs::path(node->pathFromRoot) / fs::path(materialName);
		
		try {
			fs::remove_all(pathToMaterialDirectory);
		}
		catch (fs::filesystem_error error) {
			MNEMOSY_ERROR("MaterialLibraryRegistry::DeleteMaterial: System error deleting directory \nError message: ", error.what());
			return;
		}

		//TODO:
		// once i have a selected material that is displayed in the viewport and in material editor,
		// I should switch it to a different one here and free memory;
		
		// deleting from vector
		node->subMaterialNames.erase(node->subMaterialNames.begin() + positionInVector);
		
		// maybe save library data file
	}

	void MaterialLibraryRegistry::MoveMaterial(FolderNode* sourceNode, FolderNode* targetNode, std::string& name) {

		// move material folder / copy dir and remove dir
		std::string materialName = name; // temporary storing name here
		fs::path libraryDir = m_fileDirectories.GetLibraryDirectoryPath();

		// Copying and then removing works rn but is not very elegant. fs::rename() does not work and throws acces denied error
			//fs::rename(fromPath, toPath / fromPath.filename());

		fs::path fromPath = libraryDir / fs::path(sourceNode->pathFromRoot) / fs::path(materialName);
		fs::path toPath = libraryDir / fs::path(targetNode->pathFromRoot) / fs::path(materialName);
		try { // copy directory to new location
			fs::copy(fromPath, toPath , fs::copy_options::recursive);
		}
		catch (fs::filesystem_error error) {
			MNEMOSY_ERROR("MaterialLibraryRegistry::MoveMaterial: System Error Copying directory: \nMessage: {}", error.what());
			return;
		}
		try { // remove old directory
			fs::remove_all(fromPath);
		}
		catch (fs::filesystem_error error) {
			MNEMOSY_ERROR("MaterialLibraryRegistry::MoveMaterial: System Error Removing old directory: \nMessage: {}", error.what());
			return;
		}


		// remove from source node list
		// this could maybe be done faster if we pass the vector position directly into the function
		for (int i = 0; i < sourceNode->subMaterialNames.size(); i++) {
			if (sourceNode->subMaterialNames[i] == materialName) {
				sourceNode->subMaterialNames.erase(sourceNode->subMaterialNames.begin() + i);
			}
		}

		// enlist into target node list
		targetNode->subMaterialNames.push_back(materialName);
		// save
	}

	
	
	void MaterialLibraryRegistry::LoadUserDirectoriesFromFile() {

		bool fileExists = CheckDataFile(m_userDirectoriesDataFile);
		if (!fileExists) {
			// if file does not exist we explicitly create root node here otherwise it should be created from within the file
			m_rootFolderNode = new FolderNode();
			m_rootFolderNode->name = m_rootNodeName;
			m_rootFolderNode->parent = nullptr;
			m_rootFolderNode->runtime_ID = m_runtimeIDCounter;
			m_runtimeIDCounter++;
		}

		std::string pathToDataFileString = m_userDirectoriesDataFile.path().generic_string();

		std::ifstream dataFileStream;
		dataFileStream.open(pathToDataFileString);

		nlohmann::json readFile;
		try {
			readFile = nlohmann::json::parse(dataFileStream);
		} catch (nlohmann::json::parse_error err) {
			MNEMOSY_ERROR("MaterialLibraryRegistry::LoadUserDirectoriesFromFile: Error Parsing File. Message: {}", err.what());
			return;
		}
		
		m_rootNodeName = readFile["3_UserDirectories"][m_rootNodeName]["1_name"].get<std::string>();
		bool rootIsLeaf = readFile["3_UserDirectories"][m_rootNodeName]["2_isLeaf"].get<bool>();

		FolderNode* rootFolder = CreateFolderNode(nullptr, m_rootNodeName);

		json rootJson = readFile["3_UserDirectories"][m_rootNodeName];

		m_rootFolderNode = rootFolder; 


		RecursivLoadDirectories(rootFolder, rootJson);

		dataFileStream.close();

	}

	void MaterialLibraryRegistry::SaveUserDirectoriesData() {

		std::string pathToDataFileString = m_userDirectoriesDataFile.path().generic_string();
		if (CheckDataFile(m_userDirectoriesDataFile)) {
			// if file exists we clear it first; -> really shoudl think if this is a good idea though
			std::ofstream file;
			file.open(pathToDataFileString);
			file << "";
			file.close();
		}

		std::ofstream dataFileStream;
		// start Saving
		dataFileStream.open(pathToDataFileString);
		json LibraryDirectoriesJson; // top level json object
		LibraryDirectoriesJson["1_Mnemosy_Data_File"] = "UserLibraryDirectoriesData";

		json HeaderInfo;
		std::string descriptionString = "This file stores the treelike folder structure defined by users to organise their materials";
		HeaderInfo["Description"] = descriptionString;

		LibraryDirectoriesJson["2_Header_Info"] = HeaderInfo;

		json userDirectoriesJson;

		json rootFolderJson = RecursivSaveDirectories(m_rootFolderNode);

		userDirectoriesJson[m_rootNodeName] = rootFolderJson;

		
		LibraryDirectoriesJson["3_UserDirectories"] = userDirectoriesJson;


		
		if (prettyPrintDataFile)
			dataFileStream << LibraryDirectoriesJson.dump(4);
		else
			dataFileStream << LibraryDirectoriesJson.dump(-1);

		dataFileStream.close();
	}


	// == private methods

	void MaterialLibraryRegistry::CreateNewMaterialDataFile(fs::path& folderPath, std::string& name) {
		
		fs::path materialDataFile = folderPath / fs::path(name + ".mnsydata");
		
		std::ofstream dataFileStream;
		dataFileStream.open(materialDataFile.generic_string());


		json MaterialJson; // top level json object
		MaterialJson["1_Mnemosy_Data_File"] = "Material Meta Data";
		MaterialJson["name"]		= name;

		MaterialJson["albedo_r"]	= 0.8f;
		MaterialJson["albedo_g"]	= 0.8f;
		MaterialJson["albedo_b"]	= 0.8f;

		MaterialJson["roughness"]	= 0.5f;
		MaterialJson["metallic"]	= 0.0f;

		MaterialJson["emission_r"]	= 0.0f;
		MaterialJson["emission_g"]	= 0.0f;
		MaterialJson["emission_b"]	= 0.0f;
		MaterialJson["emissionStrength"]	= 0.0f;

		MaterialJson["normalStrength"]		= 1.0f;
		MaterialJson["uvScale_x"]	= 1.0f;
		MaterialJson["uvScale_y"]	= 1.0f;

		
		MaterialJson["albedoAssigned"]		= false;
		MaterialJson["roughAssigned"]		= false;
		MaterialJson["metalAssigned"]		= false;
		MaterialJson["emissionAssigned"]	= false;
		MaterialJson["normalAssigned"]		= false;
		MaterialJson["aoAssigned"]			= false;
		
		MaterialJson["albedoPath"]		= "notAssigned";
		MaterialJson["roughPath"]		= "notAssigned";
		MaterialJson["metalPath"]		= "notAssigned";
		MaterialJson["emissionPath"]	= "notAssigned";
		MaterialJson["normalPath"]		= "notAssigned";
		MaterialJson["aoPath"]			= "notAssigned";

		fs::path thumbnailPath = fs::path(name + "_thumbnail.ktx2");
		MaterialJson["thumbnailPath"] = thumbnailPath.generic_string();


		if (prettyPrintDataFile)
			dataFileStream << MaterialJson.dump(4);
		else
			dataFileStream << MaterialJson.dump(-1);

		dataFileStream.close();
	}

	void MaterialLibraryRegistry::CreateDirectoryForNode(FolderNode* node) {

		mnemosy::core::FileDirectories& fd = MnemosyEngine::GetInstance().GetFileDirectories();
		fs::path libraryDir = fd.GetLibraryDirectoryPath();

		fs::path directoryPath = libraryDir / fs::path(node->pathFromRoot);

		fs::directory_entry newDir;
		try {
			newDir = fs::directory_entry(directoryPath);
		}
		catch (fs::filesystem_error error) {
			MNEMOSY_ERROR("MaterialLibraryRegistry::CreateDirectoryForNode: Error creating initializing Directory {}\n Error Message: {}", directoryPath.generic_string(), error.what());
			return;
		}

		if (!newDir.exists()) {
			fs::create_directories(newDir.path());
		}
		
	}

	void MaterialLibraryRegistry::RecursivUpadtePathFromRoot(FolderNode* node) {

		if (node->parent->IsRoot()) {
			node->pathFromRoot = node->name;
		}
		else {
			node->pathFromRoot = node->parent->pathFromRoot + "/" + node->name;
		}

		if (!node->IsLeafNode()) {
			for (FolderNode* child : node->subNodes) {
				RecursivUpadtePathFromRoot(child);
			}
		}
	}

	void MaterialLibraryRegistry::RecursivCleanFolderTreeMemory(FolderNode* node) {


		if (!node->subNodes.empty()) {
			for (FolderNode* subNode : node->subNodes) {
				RecursivCleanFolderTreeMemory(subNode);
			}
			node->subNodes.clear();
		} 

		if (!node->subMaterialNames.empty()) {
			node->subMaterialNames.clear();
		}

		delete node;
		node = nullptr;
	}


	void MaterialLibraryRegistry::RecursivLoadDirectories(FolderNode* node, json& jsonNode) {

		bool isLeafNode = jsonNode["2_isLeaf"].get<bool>();

		bool hasMaterials = jsonNode["6_hasMaterials"].get<bool>();
		if (hasMaterials) {
			node->subMaterialNames = jsonNode["6_materialEntries"].get<std::vector<std::string>>();
		}

		if (!isLeafNode) {

			std::vector<std::string> subFolderNames = jsonNode["4_subFolderNames"].get<std::vector<std::string>>(); // 3 nature,wood,stone
						
			for (int i = 0; i < subFolderNames.size(); i++) {

				FolderNode* subNode = new FolderNode();
				subNode->parent = node;
				subNode->name = subFolderNames[i];
				subNode->pathFromRoot = jsonNode["5_subFolders"][subFolderNames[i]]["3_pathFromRoot"].get<std::string>();
				subNode->runtime_ID = m_runtimeIDCounter;
				m_runtimeIDCounter++;
				node->subNodes.push_back(subNode);
				
				json subJson = jsonNode["5_subFolders"][subFolderNames[i]];
				RecursivLoadDirectories(subNode, subJson);
				
			}
		}
	}

	json MaterialLibraryRegistry::RecursivSaveDirectories(FolderNode* node)
	{

		json nodeJson;

		nodeJson["1_name"] = node->name;
		bool isLeafNode = node->IsLeafNode();
		nodeJson["2_isLeaf"] = isLeafNode;


		std::string pathFromRoot = "";

		if (node->parent != nullptr) { // if not root node

			if (node->parent->name == m_rootNodeName) {

				pathFromRoot = node->name;
			}
			else {
				pathFromRoot = node->parent->pathFromRoot + "/" + node->name;
			}
		}
		//MNEMOSY_DEBUG("PathFromRoot: {}", pathFromRoot);
		nodeJson["3_pathFromRoot"] = pathFromRoot;


		bool hasMaterials = !node->subMaterialNames.empty();

		nodeJson["6_hasMaterials"] = hasMaterials;
		if (hasMaterials) {
			nodeJson["6_materialEntries"] = node->subMaterialNames;
		}

		if (!isLeafNode) {
			std::vector<std::string> subNodeNames;

			for (int i = 0; i < node->subNodes.size(); i++) {

				subNodeNames.push_back(node->subNodes[i]->name);
			}
			nodeJson["4_subFolderNames"] = subNodeNames;

			json subNodes;
			for (int i = 0; i < node->subNodes.size(); i++) {

				subNodes[node->subNodes[i]->name] = RecursivSaveDirectories(node->subNodes[i]);
			}

			nodeJson["5_subFolders"] = subNodes;

		}



		return nodeJson;
	}

	bool MaterialLibraryRegistry::CheckDataFile(fs::directory_entry dataFile) {

		std::string pathToDataFileString = dataFile.path().generic_string();

		if (!dataFile.exists())
		{
			MNEMOSY_ERROR("MaterialLibraryRegistry::CheckDataFile: File did Not Exist: {} \nCreating new file at that location", pathToDataFileString);
			std::ofstream file;
			file.open(pathToDataFileString);
			file << "";
			file.close();
			return false;
		}
		if (!dataFile.is_regular_file())
		{
			MNEMOSY_ERROR("MaterialLibraryRegistry::CheckDataFile: File is not a regular file: {} \nCreating new file at that location", pathToDataFileString);
			// maybe need to delete unregular file first idk should never happen anyhow
			std::ofstream file;
			file.open(pathToDataFileString);
			file << "";
			file.close();
			return false;
		}

		return true;
	}



} // !mnemosy::systems