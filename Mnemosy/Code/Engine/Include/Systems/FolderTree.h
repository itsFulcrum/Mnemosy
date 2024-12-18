#ifndef FOLDER_TREE_H
#define FOLDER_TREE_H


#include <string>
#include <vector>
#include <json.hpp>

namespace mnemosy::systems {
	enum LibEntryType;
	struct LibEntry;
	struct FolderNode;
}

namespace mnemosy::systems {
	class FolderTree {

	public:
		FolderTree()  = default;
		~FolderTree() = default;


		void Init();
		void Shutdown();


		FolderNode& GetRoot() { return *m_rootNode; }
		FolderNode* GetRootPtr() { return m_rootNode; }
		//std::string& GetRootName() { return m_rootNodeName; }

		FolderNode* CreateNewFolder(FolderNode* parentNode, const std::string& name);
		void RenameFolder(FolderNode* node, const std::string& newName);
		void MoveFolder(FolderNode* sourceNode, FolderNode* targetParentNode);
		void DeleteFolderHierarchy(FolderNode* node);


		LibEntry* CreateNewLibEntry(FolderNode* node,const LibEntryType type,const std::string& name);

		void RenameLibEntry(LibEntry* libEntry, const std::string& name);
		void MoveLibEntry(LibEntry* libEntry, FolderNode* sourceNode, FolderNode* targetParentNode);
		void DeleteLibEntry(FolderNode* parentNode, unsigned int posInVector);


		bool IsLibEntryWithinHierarchy(FolderNode* hierarchyRoot, LibEntry* libEnry);
		bool IsNodeWithinHierarchy(FolderNode* hierarchyRoot, FolderNode* node);

		bool CollectMaterialsFromSearchKeyword(const std::string& searchKeyword);

		FolderNode* RecursivGetNodeByID(FolderNode* startNode, const unsigned int id);


		std::string MakeNameUnique(const std::string& name);

		void LoadFromJson(nlohmann::json& rootJson);
		nlohmann::json* WriteToJson();
		void Clear();

		std::vector<systems::LibEntry*>& GetSearchResultsList() { return m_searchResults; }


		unsigned int RecursiveCountMaterials(FolderNode* node, const unsigned int startValue);
	private:

		LibEntry* CreateMaterial_Internal(FolderNode* node, LibEntryType type,const std::string name);
		FolderNode* CreateNewFolder_Internal(FolderNode* parentNode, const std::string& name);


		void RecursivDeleteHierarchy(FolderNode* node);
		//void RecursivUpdatePathFromRoot(FolderNode* node);

		bool RecursivDoesNameExist(FolderNode* node, const std::string& name);

		void RecursivCollectMaterialSearch(FolderNode* node, const std::string& searchKeyword);

		nlohmann::json RecursivWriteToJson(FolderNode* node);
		void RecursivLoadFromJson(FolderNode* node, const nlohmann::json& jsonNode);


	private:
		unsigned int m_runtimeIDCounter;
		unsigned int m_runtimeMaterialIDCounter;

		//std::string m_treeName;
		FolderNode* m_rootNode = nullptr;

		std::vector<LibEntry*> m_searchResults;

	}; 


} // namespace mnemosy::systems

#endif // !FOLDER_TREE_H
