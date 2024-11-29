#include "Include/Systems/FolderTree.h"
#include "Include/Core/Log.h"

#include "Include/Core/Utils/StringUtils.h"
#include "Include/Systems/FolderTreeNode.h"
#include "Include/Systems/JsonKeys.h"

namespace mnemosy::systems {

	void FolderTree::Init(std::string rootName) {
		m_runtimeIDCounter = 1;
		m_runtimeMaterialIDCounter = 1;
		m_rootNodeName = std::string(rootName);

		m_rootNode = CreateNewFolder_Internal(nullptr, m_rootNodeName);

	}

	void FolderTree::Shutdown() {
		RecursivDeleteHierarchy(m_rootNode);

		if (!m_searchResults.empty()) {
			m_searchResults.clear();
		}
	}

	FolderNode* FolderTree::CreateNewFolder(FolderNode* parentNode, const std::string& name) {

		return CreateNewFolder_Internal(parentNode, MakeNameUnique(name));
	}

	void FolderTree::RenameFolder(FolderNode* node, const std::string& newName) {

		node->name = MakeNameUnique(newName);
		RecursivUpdatePathFromRoot(node);
	}

	bool FolderTree::CollectMaterialsFromSearchKeyword(const std::string& searchKeyword) {

		if (!m_searchResults.empty()) {
			m_searchResults.clear();
		}

		std::string keyword = searchKeyword;

		core::StringUtils::MakeStringLowerCase(keyword);

		RecursivCollectMaterialSearch(m_rootNode, keyword);


		if (!m_searchResults.empty()) {
			return true;
		}


		return false;
	}

	FolderNode* FolderTree::RecursivGetNodeByID(FolderNode* startNode, const unsigned int id)
	{
		if (startNode == nullptr) {
			return nullptr;
		}

		if (startNode->runtime_ID == id) {
			return startNode;
		}

		if (startNode->IsLeafNode()) {
			return nullptr;
		}

		FolderNode* foundNode = nullptr;

		for (size_t i = 0; i < startNode->subNodes.size(); i++) {
			foundNode = RecursivGetNodeByID(startNode->subNodes[i], id);
			if (foundNode != nullptr) {
				break;
			}
		}

		return foundNode;
	}

	void FolderTree::DeleteFolderHierarchy(FolderNode* node){

		// remove node from parents subnodes list
		FolderNode* parent = node->parent;
		for (size_t i = 0; i < parent->subNodes.size(); i++) {
			if (parent->subNodes[i]->runtime_ID == node->runtime_ID) {
				parent->subNodes.erase(parent->subNodes.begin() + i);
				break;
			}
		}
		RecursivDeleteHierarchy(node);
	}

	LibEntry* FolderTree::CreateNewLibEntry(FolderNode* node, const LibEntryType type,const std::string& name){

		return CreateMaterial_Internal(node, type, MakeNameUnique( name));
	}

	void FolderTree::RenameLibEntry(LibEntry* libEntry, const std::string& name) {

		libEntry->name = MakeNameUnique( name);
	}

	void FolderTree::MoveLibEntry(LibEntry* libEntry, FolderNode* sourceNode, FolderNode* targetParentNode) {

		targetParentNode->subEntries.push_back(libEntry);
		libEntry->parent = targetParentNode;

		for (size_t i = 0; i < sourceNode->subEntries.size(); i++) {

			if (sourceNode->subEntries[i]->runtime_ID == libEntry->runtime_ID) {

				sourceNode->subEntries.erase(sourceNode->subEntries.begin() + i);
				break;
			}
		}
	}

	void FolderTree::DeleteLibEntry(FolderNode* parentNode, unsigned int posInVector) {

		LibEntry* mat = parentNode->subEntries[posInVector];
		parentNode->subEntries.erase(parentNode->subEntries.begin() + posInVector);

		delete mat;
	}

	bool FolderTree::IsLibEntryWithinHierarchy(FolderNode* hierarchyRoot, LibEntry* libEnry) {
		MNEMOSY_ASSERT(hierarchyRoot && libEnry, "They cannot be null!");

		// walk up parents of libEntry until root node is reached and check if parent is equal to the hierarchy root
		FolderNode* curr = libEnry->parent;
		while (!curr->IsRoot()) {

			if (curr->runtime_ID == hierarchyRoot->runtime_ID) {
				return true;
			}

			curr = curr->parent;
		}

		return false;
	}

	bool FolderTree::IsNodeWithinHierarchy(FolderNode* hierarchyRoot, FolderNode* node) {
		MNEMOSY_ASSERT(hierarchyRoot && node, "They cannot be null!");
		
		// walk up parents of node until root node is reached and check if parent is equal to the hierarchy root		
		FolderNode* curr = node;
		while (!curr->IsRoot()) {

			if (curr->runtime_ID == hierarchyRoot->runtime_ID) {
				return true;
			}
			curr = curr->parent;
		}

		return false;
	}

	void FolderTree::MoveFolder(FolderNode* sourceNode, FolderNode* targetParentNode) {

		// just to clarify:
		// source node is the folder we want to move and targetParentNode is the new destination folder

		// remove from parents children
		FolderNode* oldParent = sourceNode->parent;
		for (int i = 0; i < oldParent->subNodes.size(); i++) {

			if (oldParent->subNodes[i]->name == sourceNode->name) {
				oldParent->subNodes.erase(oldParent->subNodes.begin() + i);
			}
		}

		// set new parent
		sourceNode->parent = targetParentNode;
		// add as child of new parent
		targetParentNode->subNodes.push_back(sourceNode);

		RecursivUpdatePathFromRoot(sourceNode);
	}

	void FolderTree::Clear(){
		// clears the entire tree but not the root node

		if (!m_rootNode->IsLeafNode()) {
			for (size_t i = 0; i < m_rootNode->subNodes.size(); i++) {

				RecursivDeleteHierarchy(m_rootNode->subNodes[i]);
			}
			m_rootNode->subNodes.clear();
		}

		if (m_rootNode->HasMaterials()) {

			for (int i = 0; i < m_rootNode->subEntries.size(); i++) {

				delete m_rootNode->subEntries[i];
			}

			m_rootNode->subEntries.clear();
		}
	}

	std::string FolderTree::MakeNameUnique(const std::string& name) {

		// ensure that the name is completely unique across all folders and materials
		std::string baseName = name;
		std::string lowerBaseName = core::StringUtils::ToLowerCase(name);
		unsigned int suffixNbr = 0;

		std::string uniqueName = baseName;
		std::string lowerUniqueName = lowerBaseName;

		while (RecursivDoesNameExist(m_rootNode, lowerUniqueName)) {
			suffixNbr++;
			uniqueName = baseName + "_" + std::to_string(suffixNbr);
			lowerUniqueName = core::StringUtils::ToLowerCase(uniqueName);
		}

		return uniqueName;
	}

	void FolderTree::LoadFromJson(nlohmann::json& rootJson) {

		//bool rootIsLeaf = rootJson["3_UserDirectories"][m_rootNodeName]["2_isLeaf"].get<bool>();

		nlohmann::json firstTreeEntryJson = rootJson["3_UserDirectories"][m_rootNodeName];

		RecursivLoadFromJson(m_rootNode, firstTreeEntryJson);
	}

	nlohmann::json* FolderTree::WriteToJson() {

		nlohmann::json LibraryDirectoriesJson; // top level json object
		LibraryDirectoriesJson[jsonLibKey_MnemosyDataFile] = "UserLibraryDirectoriesData";

		nlohmann::json HeaderInfo;
		std::string descriptionString = "!!! == DO NOT DELETE, MOVE, OR MODIFY THIS FILE ==!!! This file stores the treelike folder structure defined by users to organise their materials";
		HeaderInfo[jsonLibKey_Description] = descriptionString;

		LibraryDirectoriesJson[jsonLibKey_HeaderInfo] = HeaderInfo;


		nlohmann::json rootFolderJson = RecursivWriteToJson(m_rootNode);

		nlohmann::json userDirectoriesJson;
		userDirectoriesJson[m_rootNodeName] = rootFolderJson;
		LibraryDirectoriesJson[jsonLibKey_FolderTree] = userDirectoriesJson;

		return new nlohmann::json(LibraryDirectoriesJson);
	}

	unsigned int FolderTree::RecursiveCountMaterials(FolderNode* node, const unsigned int startValue) {

		unsigned int value = startValue;

		if (node->HasMaterials()) {
			value += node->subEntries.size();
		}

		if (!node->IsLeafNode()) {

			for (int i = 0; i < node->subNodes.size(); i++) {

				value = RecursiveCountMaterials(node->subNodes[i], value);	
			}
		}

		return value;
	}

	LibEntry* FolderTree::CreateMaterial_Internal(FolderNode* node, const LibEntryType type, const std::string name) {

		// adding entry to list of directory node;
		LibEntry* matInfo = new LibEntry();
		matInfo->type = type;
		matInfo->name = name;
		matInfo->parent = node;
		matInfo->runtime_ID = m_runtimeMaterialIDCounter;
		m_runtimeMaterialIDCounter++;
		matInfo->selected = false;
		matInfo->thumbnailTexure_ID = 0;
		matInfo->thumbnailLoaded = false;

		node->subEntries.push_back(matInfo);

		return matInfo;
	}

	FolderNode* FolderTree::CreateNewFolder_Internal(FolderNode* parentNode, const std::string& name) {
		FolderNode* node = new FolderNode();
		node->name = name;
		node->parent = parentNode;
		node->runtime_ID = m_runtimeIDCounter;
		m_runtimeIDCounter++;

		if (parentNode != nullptr) { // if the new node is not the root node

			parentNode->subNodes.push_back(node);

			if (parentNode->IsRoot()) {
				node->pathFromRoot = std::filesystem::path(node->name);
			}
			else {
				node->pathFromRoot = parentNode->pathFromRoot / std::filesystem::path(node->name);
			}
		}

		return node;
	}

	void FolderTree::RecursivDeleteHierarchy(FolderNode* node) {

		if (!node->IsLeafNode()) {
			for (FolderNode* subNode : node->subNodes) {
				RecursivDeleteHierarchy(subNode);
			}
			node->subNodes.clear();
		}

		if (node->HasMaterials()) {

			for (int i = 0; i < node->subEntries.size(); i++) {
				delete node->subEntries[i];
			}
			node->subEntries.clear();
		}

		delete node;
		node = nullptr;
	}

	void FolderTree::RecursivUpdatePathFromRoot(FolderNode* node) {

		if (node->parent->IsRoot()) {
			node->pathFromRoot = std::filesystem::path(node->name);
		}
		else {
			node->pathFromRoot = node->parent->pathFromRoot / std::filesystem::path(node->name);
		}

		if (!node->IsLeafNode()) {
			for (FolderNode* child : node->subNodes) {
				RecursivUpdatePathFromRoot(child);
			}
		}
	}

	bool FolderTree::RecursivDoesNameExist(FolderNode* node, const std::string& name) {

		// recursivly walk through the entire tree and check if the name already exists somewhere either as a folder or as a material
		// returns true if the name exists
		// input name is assumed to be lower case to avoid doing this computation multiple times

		std::string lowerName = name; //core::StringUtils::ToLowerCase(name); // we pass always a lower case already
		std::string currentNodeName = core::StringUtils::ToLowerCase(node->name);

		// Check current node
		if (currentNodeName == lowerName) {
			return true;
		}


		// check sub entries names
		if (node->HasMaterials()) {

			for (int i = 0; i < node->subEntries.size(); i++) {

				if (core::StringUtils::ToLowerCase(node->subEntries[i]->name) == lowerName) {
					return true;
				}
			}
		}

		// check sub folders
		if (!node->IsLeafNode()) {

			for (FolderNode* subnode : node->subNodes) {

				if (RecursivDoesNameExist(subnode, lowerName)) {
					return true;
				}
			}
		}

		return false;
	}

	void FolderTree::RecursivCollectMaterialSearch(FolderNode* node, const std::string& searchKeyword) {


		if (node->HasMaterials()) {

			// collect suitable materials

			for (int i = 0; i < node->subEntries.size(); i++) {

				std::string matName = node->subEntries[i]->name;
				core::StringUtils::MakeStringLowerCase(matName);

				if (matName.find(searchKeyword) != std::string::npos) {

					// Found a material that contains the keyword in its name

					m_searchResults.push_back(node->subEntries[i]);

				}
			}
		}


		if (!node->IsLeafNode()) {


			for (int i = 0; i < node->subNodes.size(); i++) {

				RecursivCollectMaterialSearch(node->subNodes[i], searchKeyword);
			}

		}

	}

	nlohmann::json FolderTree::RecursivWriteToJson(FolderNode* node) {

		nlohmann::json nodeJson;

		
		nodeJson[jsonLibKey_name] = node->name;

		bool isLeafNode = node->IsLeafNode();
		nodeJson[jsonLibKey_isLeaf] = isLeafNode;

		std::string pathFromRoot = "";
		if (node->parent != nullptr) { // if not root node

			if (node->parent->IsRoot()) {

				pathFromRoot = node->name;
			}
			else {
				std::filesystem::path fs_path = node->parent->pathFromRoot / std::filesystem::path(node->name);
				pathFromRoot = fs_path.generic_string();
			}
		}

		nodeJson[jsonLibKey_pathFromRoot] = pathFromRoot;

		bool hasMaterials = !node->subEntries.empty();

		nodeJson[jsonLibKey_hasMaterials] = hasMaterials;
		if (hasMaterials) {

			std::vector<std::string> entryNames;
			std::vector<int> entryTypes;

			for (size_t i = 0; i < node->subEntries.size(); i++) {
				entryNames.push_back(node->subEntries[i]->name);
				entryTypes.push_back((int)node->subEntries[i]->type);
			}

			nodeJson[jsonLibKey_materialEntries] = entryNames;
			nodeJson[jsonLibKey_entryTypes] = entryTypes;

			entryNames.clear();
		}

		if (!isLeafNode) {
			std::vector<std::string> subNodeNames;

			for (size_t i = 0; i < node->subNodes.size(); i++) {

				subNodeNames.push_back(node->subNodes[i]->name);
			}
			nodeJson[jsonLibKey_subFolderNames] = subNodeNames;

			nlohmann::json subNodes;
			for (size_t i = 0; i < node->subNodes.size(); i++) {

				subNodes[node->subNodes[i]->name] = RecursivWriteToJson(node->subNodes[i]);
			}

			nodeJson[jsonLibKey_subFolders] = subNodes;
		}

		return nodeJson;
	}

	void FolderTree::RecursivLoadFromJson(FolderNode* node, const nlohmann::json& jsonNode) {

		bool isLeafNode = jsonNode[jsonLibKey_isLeaf].get<bool>();

		bool hasMaterials = jsonNode[jsonLibKey_hasMaterials].get<bool>();
		if (hasMaterials) {
			std::vector<std::string> subMatNames = jsonNode[jsonLibKey_materialEntries].get<std::vector<std::string>>();
			
			std::vector<int> subEntryTypes;
			if (jsonNode.contains(jsonLibKey_entryTypes)) {
				subEntryTypes = jsonNode[jsonLibKey_entryTypes].get<std::vector<int>>();
			}
			else {
				MNEMOSY_WARN("Could not find entryTypes - maybe older version of mnemosy file. initializing all types as pbr materials");
				subEntryTypes = std::vector<int>(subMatNames.size(), (int)systems::LibEntryType::MNSY_ENTRY_TYPE_PBRMAT);
			}
			
			MNEMOSY_ASSERT(subEntryTypes.size() == subMatNames.size(), "Something broke in the saved data file. Names and entryType lists must have equal size");

			for (size_t i = 0; i < subMatNames.size(); i++) {

				CreateMaterial_Internal(node, (systems::LibEntryType)subEntryTypes[i] , subMatNames[i]);
			}

			node->SortLibEntries();
		}

		if (!isLeafNode) {

			std::vector<std::string> subFolderNames = jsonNode[jsonLibKey_subFolderNames].get<std::vector<std::string>>();

			mnemosy::core::StringUtils::SortVectorListAlphabetcially(subFolderNames);

			for (size_t i = 0; i < subFolderNames.size(); i++) {

				FolderNode* subNode = CreateNewFolder_Internal(node, subFolderNames[i]);
				nlohmann::json subJson = jsonNode[jsonLibKey_subFolders][subFolderNames[i]];
				RecursivLoadFromJson(subNode, subJson);
			}
		}

	}

} // namespace mnemosy::systems