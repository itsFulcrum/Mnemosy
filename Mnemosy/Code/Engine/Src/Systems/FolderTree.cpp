#include "Include/Systems/FolderTree.h"
#include "Include/Core/Log.h"

#include "Include/Systems/FolderTreeNode.h"
#include "Include/Systems/JsonKeys.h"


namespace mnemosy::systems {

	FolderTree::FolderTree(std::string rootName)
		: m_runtimeIDCounter{1}
		, m_runtimeMaterialIDCounter{1}
		, m_rootNodeName{rootName}
	{
		m_rootNode = CreateNewFolder_Internal(nullptr, m_rootNodeName);
	}


	FolderTree::~FolderTree()
	{
		RecursivDeleteHierarchy(m_rootNode);
	}

	FolderNode* FolderTree::CreateNewFolder(FolderNode* parentNode, const std::string& name) {

		return CreateNewFolder_Internal(parentNode, MakeNameUnique(name));
	}

	void FolderTree::RenameFolder(FolderNode* node, const std::string& newName) {

		node->name = MakeNameUnique(newName);
		RecursivUpdatePathFromRoot(node);
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
			}
		}
		RecursivDeleteHierarchy(node);
	}

	MaterialInfo& FolderTree::CreateNewMaterial(FolderNode* node, const std::string& name){

		return CreateMaterial_Internal(node,  MakeNameUnique( name));
	}

	void FolderTree::RenameMaterial(MaterialInfo& materialInfo, const std::string& name) {
		materialInfo.name = MakeNameUnique( name);
	}

	void FolderTree::MoveMaterial(MaterialInfo materialInfo, FolderNode* sourceNode, FolderNode* targetParentNode) {

		targetParentNode->subMaterials.push_back(materialInfo);

		for (size_t i = 0; i < sourceNode->subMaterials.size(); i++) {

			if (sourceNode->subMaterials[i].runtime_ID == materialInfo.runtime_ID) {

				sourceNode->subMaterials.erase(sourceNode->subMaterials.begin() + i);
				break;
			}
		}
	}

	void FolderTree::DeleteMaterial(FolderNode* parentNode, unsigned int posInVector) {

		parentNode->subMaterials.erase(parentNode->subMaterials.begin() + posInVector);
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
			m_rootNode->subMaterials.clear();
		}
	}

	std::string FolderTree::MakeNameUnique(const std::string& name) {

		// ensure that the name is completely unique across all folders and materials


		unsigned int suffixNbr = 0;

		std::string newName = name;

		while (RecursivDoesNameExist(m_rootNode, newName)) {
			suffixNbr++;
			newName = name + "_" + std::to_string(suffixNbr);
		}

		return newName;
	}

	void FolderTree::LoadFromJson(nlohmann::json& rootJson) {

		bool rootIsLeaf = rootJson["3_UserDirectories"][m_rootNodeName]["2_isLeaf"].get<bool>();

		nlohmann::json firstTreeEntryJson = rootJson["3_UserDirectories"][m_rootNodeName];

		RecursivLoadFromJson(m_rootNode, firstTreeEntryJson);
	}

	nlohmann::json* FolderTree::WriteToJson() {

		nlohmann::json LibraryDirectoriesJson; // top level json object
		LibraryDirectoriesJson[jsonLibKey_MnemosyDataFile] = "UserLibraryDirectoriesData";

		nlohmann::json HeaderInfo;
		std::string descriptionString = "This file stores the treelike folder structure defined by users to organise their materials";
		HeaderInfo[jsonLibKey_Description] = descriptionString;

		LibraryDirectoriesJson[jsonLibKey_HeaderInfo] = HeaderInfo;


		nlohmann::json rootFolderJson = RecursivWriteToJson(m_rootNode);

		nlohmann::json userDirectoriesJson;
		userDirectoriesJson[m_rootNodeName] = rootFolderJson;
		LibraryDirectoriesJson[jsonLibKey_FolderTree] = userDirectoriesJson;

		return new nlohmann::json(LibraryDirectoriesJson);
	}

	MaterialInfo& FolderTree::CreateMaterial_Internal(FolderNode* node, const std::string name) {

		//// adding entry to list of directory node;
		MaterialInfo matInfo;
		matInfo.name = name;
		matInfo.runtime_ID = m_runtimeMaterialIDCounter;
		m_runtimeMaterialIDCounter++;
		matInfo.selected = false;
		matInfo.thumbnailTexure_ID = 0;
		matInfo.thumbnailLoaded = false;

		node->subMaterials.push_back(matInfo);

		return node->subMaterials.back();
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
			node->subMaterials.clear();
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
		
		if (node->name == name) {

			return true;
		}


		if (node->HasMaterials()) {


			for (size_t i = 0; i < node->subMaterials.size(); i++) {

				if (node->subMaterials[i].name == name) {
					return true;
				}
			}
		}

		if (!node->IsLeafNode()) {

			std::string n = name;
			if (node->SubnodeExistsAlready(n)) {

				return true;
			}

			for (size_t i = 0; i < node->subNodes.size(); i++) {


				bool foundMatch = RecursivDoesNameExist(node->subNodes[i], name);

				if (foundMatch) {

					return true;
				}

			}
		}

		return false;
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

		bool hasMaterials = !node->subMaterials.empty();

		nodeJson[jsonLibKey_hasMaterials] = hasMaterials;
		if (hasMaterials) {

			std::vector<std::string> matNames;
			for (size_t i = 0; i < node->subMaterials.size(); i++) {
				matNames.push_back(node->subMaterials[i].name);
			}

			nodeJson[jsonLibKey_materialEntries] = matNames;
			matNames.clear();
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
			for (size_t i = 0; i < subMatNames.size(); i++) {

				CreateMaterial_Internal(node, subMatNames[i]);
			}
		}

		if (!isLeafNode) {

			std::vector<std::string> subFolderNames = jsonNode[jsonLibKey_subFolderNames].get<std::vector<std::string>>();

			for (size_t i = 0; i < subFolderNames.size(); i++) {

				FolderNode* subNode = CreateNewFolder_Internal(node, subFolderNames[i]);
				nlohmann::json subJson = jsonNode[jsonLibKey_subFolders][subFolderNames[i]];
				RecursivLoadFromJson(subNode, subJson);
			}
		}

	}

} // namespace mnemosy::systems