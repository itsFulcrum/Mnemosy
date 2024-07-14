#ifndef FOLDER_TREE_H
#define FOLDER_TREE_H

#include <nlohmann/json.hpp>
using namespace nlohmann;

#include "string"

namespace mnemosy::systems {
	struct MaterialInfo;
	struct FolderNode;
	struct JsonLibKeys;
}

namespace mnemosy::systems {
	class FolderTree {

	public:
		FolderTree(std::string rootName);
		~FolderTree();

		FolderNode& GetRoot() { return *m_rootNode; }
		FolderNode* GetRootPtr() { return m_rootNode; }
		std::string& GetRootName() { return m_rootNodeName; }

		FolderNode* CreateNewFolder(FolderNode* parentNode, const std::string& name);
		void RenameFolder(FolderNode* node, const std::string& newName);
		void MoveFolder(FolderNode* sourceNode, FolderNode* targetParentNode);
		void DeleteFolderHierarchy(FolderNode* node);


		MaterialInfo& CreateNewMaterial(FolderNode* node,const std::string& name);
		void RenameMaterial(MaterialInfo& materialInfo, const std::string& name);
		void MoveMaterial(MaterialInfo materialInfo, FolderNode* sourceNode, FolderNode* targetParentNode);
		void DeleteMaterial(FolderNode* parentNode, unsigned int posInVector);

		FolderNode* RecursivGetNodeByID(FolderNode* startNode, const unsigned int id);


		std::string MakeNameUnique(const std::string& name);

		void LoadFromJson(json& rootJson);
		json* WriteToJson();
		void Clear();

	private:
		MaterialInfo& CreateMaterial_Internal(FolderNode* node,const std::string name);
		FolderNode* CreateNewFolder_Internal(FolderNode* parentNode, const std::string& name);

		void RecursivDeleteHierarchy(FolderNode* node);
		void RecursivUpdatePathFromRoot(FolderNode* node);

		unsigned int RecursivCheckNames(FolderNode* node, const std::string& name, const unsigned int suffixNbr);


		json RecursivWriteToJson(FolderNode* node);
		void RecursivLoadFromJson(FolderNode* node, const json& jsonNode);

		unsigned int m_runtimeIDCounter;
		unsigned int m_runtimeMaterialIDCounter;

		std::string m_rootNodeName;
		FolderNode* m_rootNode = nullptr;

		JsonLibKeys* m_jsonLibKeys = nullptr;
	}; 


} // namespace mnemosy::systems

#endif // !FOLDER_TREE_H
