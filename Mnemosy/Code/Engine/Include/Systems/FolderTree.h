#ifndef FOLDER_TREE_H
#define FOLDER_TREE_H


#include <string>
#include <vector>
#include <nlohmann/json.hpp>

namespace mnemosy::systems {
	struct MaterialInfo;
	struct FolderNode;
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


		MaterialInfo* CreateNewMaterial(FolderNode* node,const std::string& name);
		void RenameMaterial(MaterialInfo* materialInfo, const std::string& name);
		void MoveMaterial(MaterialInfo* materialInfo, FolderNode* sourceNode, FolderNode* targetParentNode);
		void DeleteMaterial(FolderNode* parentNode, unsigned int posInVector);


		bool CollectMaterialsFromSearchKeyword(const std::string& searchKeyword);

		FolderNode* RecursivGetNodeByID(FolderNode* startNode, const unsigned int id);


		std::string MakeNameUnique(const std::string& name);

		void LoadFromJson(nlohmann::json& rootJson);
		nlohmann::json* WriteToJson();
		void Clear();

		std::vector<systems::MaterialInfo*>& GetSearchResultsList() { return m_searchResults; }


		unsigned int RecursiveCountMaterials(FolderNode* node, const unsigned int startValue);
	private:
		MaterialInfo* CreateMaterial_Internal(FolderNode* node,const std::string name);
		FolderNode* CreateNewFolder_Internal(FolderNode* parentNode, const std::string& name);

		void RecursivDeleteHierarchy(FolderNode* node);
		void RecursivUpdatePathFromRoot(FolderNode* node);

		bool RecursivDoesNameExist(FolderNode* node, const std::string& name);

		void RecursivCollectMaterialSearch(FolderNode* node, const std::string& searchKeyword);

		nlohmann::json RecursivWriteToJson(FolderNode* node);
		void RecursivLoadFromJson(FolderNode* node, const nlohmann::json& jsonNode);


		void MakeStringLowerCase(std::string& str);



	private:
		unsigned int m_runtimeIDCounter;
		unsigned int m_runtimeMaterialIDCounter;

		std::string m_rootNodeName;
		FolderNode* m_rootNode = nullptr;

		std::vector<MaterialInfo*> m_searchResults;

	}; 


} // namespace mnemosy::systems

#endif // !FOLDER_TREE_H
