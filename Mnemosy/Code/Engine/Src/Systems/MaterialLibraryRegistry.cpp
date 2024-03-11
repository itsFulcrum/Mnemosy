#include "Include/Systems/MaterialLibraryRegistry.h"

#include "Include/Core/Log.h"
#include "Include/MnemosyEngine.h"
#include "Include/Core/FileDirectories.h"

#include <fstream>

namespace mnemosy::systems
{
	// public methods
 
	MaterialLibraryRegistry::MaterialLibraryRegistry() {

		m_runtimeIDCounter = 0;
		mnemosy::core::FileDirectories& fd = MnemosyEngine::GetInstance().GetFileDirectories();
		fs::path pathToUserDirectoriesDataFile = fd.GetDataPath() / fs::path("UserLibraryDirectories.mnsydata");
		m_userDirectoriesDataFile = fs::directory_entry(pathToUserDirectoriesDataFile); 
		
		
		LoadUserDirectoriesFromFile();

	}

	MaterialLibraryRegistry::~MaterialLibraryRegistry() {
		
		SaveUserDirectoriesData();
		// recursivly cleanup folderTree heap memory
		RecursivCleanFolderTreeMemory(m_rootFolderNode);
	}

	FolderNode* MaterialLibraryRegistry::GetRootFolder() {

		return m_rootFolderNode;
	}

	FolderNode* MaterialLibraryRegistry::CreateFolderNode(FolderNode* parentNode, std::string name) {
		FolderNode* node = new FolderNode();
		node->name = name;
		node->parent = parentNode;

		std::string pathFromRoot = "";
		if (parentNode != nullptr) { // if this not root node
			if (parentNode->name == "Root") { // if parent is root
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

		userDirectoriesJson["Root"] = rootFolderJson;

		
		LibraryDirectoriesJson["3_UserDirectories"] = userDirectoriesJson;


		
		if (prettyPrintDataFile)
			dataFileStream << LibraryDirectoriesJson.dump(4);
		else
			dataFileStream << LibraryDirectoriesJson.dump(-1);

		dataFileStream.close();
	}

	void MaterialLibraryRegistry::LoadUserDirectoriesFromFile() {

		bool fileExists = CheckDataFile(m_userDirectoriesDataFile);
		if (!fileExists) {
			// if file does not exist we explicitly create root node here otherwise it should be created from within the file
			m_rootFolderNode = new FolderNode();
			m_rootFolderNode->name = "Root";
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

		std::string rootName = readFile["3_UserDirectories"]["Root"]["1_name"].get<std::string>();
		bool rootIsLeaf = readFile["3_UserDirectories"]["Root"]["2_isLeaf"].get<bool>();

		FolderNode* rootFolder = CreateFolderNode(nullptr, rootName);

		json rootJson = readFile["3_UserDirectories"]["Root"];

		m_rootFolderNode = rootFolder; 


		RecursivLoadDirectories(rootFolder, rootJson);

		dataFileStream.close();

	}

	void MaterialLibraryRegistry::DeleteFolderHierarchy(FolderNode* node)
	{
		{
			// delete directories from disk
			fs::path libraryDir = MnemosyEngine::GetInstance().GetFileDirectories().GetLibraryDirectoryPath();

			fs::path directoryPathToDelete = libraryDir / fs::path(node->pathFromRoot);

			try {
				// this call removes all files and directories underneith permanently without moving it to trashbin
				fs::remove_all(directoryPathToDelete);
			}
			catch (fs::filesystem_error error) {
				MNEMOSY_ERROR("MaterialLibraryRegistry::DeleteFolderHierarchy: Error Deleting path: {} \nError Message: {} ", directoryPathToDelete.generic_string(), error.what());
			}
		}

		if (node->parent == nullptr || node->name == "root")
		{
			MNEMOSY_WARN("you cannot delete the root directory");
			return;
		}

		for (int i = 0; i < node->parent->subNodes.size(); i++)
		{
			if (node->parent->subNodes[i]->name == node->name) {

				node->parent->subNodes.erase(node->parent->subNodes.begin() + i);
			}
		}

		RecursivCleanFolderTreeMemory(node);

	}

	void MaterialLibraryRegistry::RenameDirectory(FolderNode* node, std::string oldPathFromRoot) {
		
		mnemosy::core::FileDirectories& fd = MnemosyEngine::GetInstance().GetFileDirectories();
		fs::path libraryDir = fd.GetLibraryDirectoryPath();

		fs::path oldPath = libraryDir / fs::path(oldPathFromRoot);
		fs::path newPath = libraryDir / fs::path(node->pathFromRoot);

		try {
			fs::rename(oldPath, newPath);
		}
		catch (fs::filesystem_error error) {
			MNEMOSY_ERROR("MaterialLibraryRegistry::RenameDirectory: Error Renaming file: {} to: {} \nError message: {}", oldPath.generic_string(), newPath.generic_string(), error.what());
		}
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
		}
		try { // remove old directory
			fs::remove_all(fromPath);
		}
		catch (fs::filesystem_error error) {
			MNEMOSY_ERROR("MaterialLibraryRegistry::MoveDirectory: System Error Removing old directory: \nMessage: {}", error.what());
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
		if (dragSource->parent->name == "Root") {
			dragSource->pathFromRoot = dragSource->name; 
		}
		else {
			dragSource->pathFromRoot = dragSource->parent->pathFromRoot + "/" + dragSource->name; 
		}

		// should prob save to data
		SaveUserDirectoriesData();
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

	void MaterialLibraryRegistry::RecursivLoadDirectories(FolderNode* node, json& jsonNode) {

		bool isLeafNode = jsonNode["2_isLeaf"].get<bool>();
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

			if (node->parent->name == "Root") {

				pathFromRoot = node->name;
			}
			else {
				pathFromRoot = node->parent->pathFromRoot + "/" + node->name;
			}
		}
		//MNEMOSY_DEBUG("PathFromRoot: {}", pathFromRoot);
		nodeJson["3_pathFromRoot"] = pathFromRoot;

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

	// == private methods

	void MaterialLibraryRegistry::RecursivCleanFolderTreeMemory(FolderNode* node) {


		if (!node->subNodes.empty()) {
			for (FolderNode* subNode : node->subNodes) {
				RecursivCleanFolderTreeMemory(subNode);
			}
			node->subNodes.clear();
		} 
		delete node;
		node = nullptr;
	}

	// methos for struct
	FolderNode* FolderNode::GetSubNodeByName(std::string name)
	{

		if (!subNodes.empty()) {

			for (FolderNode* node : subNodes) {

				if (node->name == name) {
					return node;
				}
			}
		}
		return nullptr;

	}

	bool FolderNode::IsLeafNode()
	{

		if (subNodes.empty())
		{
			return true;
		}

		return false;
	}

	bool FolderNode::SubnodeExistsAlready(FolderNode* node, std::string name)	{
		// only searches curent and subnodes

		if (node->name == name)
			return true;

		if (!subNodes.empty()) {

			for (FolderNode* subnode : node->subNodes) {

				if (subnode->name == name) {
					return true;
				}
			}
		}

		return false;
	}

} // !mnemosy::systems