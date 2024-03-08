#include "Include/Systems/MaterialLibraryRegistry.h"

#include "Include/Core/Log.h"
#include "Include/MnemosyEngine.h"
#include "Include/Core/FileDirectories.h"

#include <fstream>



namespace mnemosy::systems
{
	// public methods
 
	MaterialLibraryRegistry::MaterialLibraryRegistry() {


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

	void MaterialLibraryRegistry::RecursivLoadDirectories(FolderNode* node, json& jsonNode)
	{
		//std::string name = jsonNode["1_name"].get<std::string>();
		bool isLeafNode = jsonNode["2_isLeaf"].get<bool>();
		


		if (!isLeafNode) {

			std::vector<std::string> subFolderNames = jsonNode["3_subFolderNames"].get<std::vector<std::string>>(); // 3 nature,wood,stone
						
			for (int i = 0; i < subFolderNames.size(); i++) {

				FolderNode* subNode = CreateFolderNode(node, subFolderNames[i]);
				node->subNodes.push_back(subNode);
				json subJson = jsonNode["4_subFolders"][subFolderNames[i]];
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

		if (!isLeafNode) {
			std::vector<std::string> subNodeNames;

			for (int i = 0; i < node->subNodes.size(); i++) {

				subNodeNames.push_back(node->subNodes[i]->name);
			}
			nodeJson["3_subFolderNames"] = subNodeNames;

			json subNodes;
			for (int i = 0; i < node->subNodes.size(); i++) {

				subNodes[node->subNodes[i]->name] = RecursivSaveDirectories(node->subNodes[i]);
			}

			nodeJson["4_subFolders"] = subNodes;

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