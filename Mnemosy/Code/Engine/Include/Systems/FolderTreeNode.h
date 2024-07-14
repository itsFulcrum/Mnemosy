#ifndef FOLDER_TREE_NODE_H
#define FOLDER_TREE_NODE_H


#include <vector>
#include <string>
#include <filesystem>


namespace mnemosy::systems
{
	struct MaterialInfo {
	public:
		std::string name;
		unsigned int runtime_ID; // only used for runtime identification.
		bool thumbnailLoaded = false;
		unsigned int thumbnailTexure_ID = 0;
	};


	struct FolderNode {
	public:
		unsigned int runtime_ID; // only used for runtime identification.
		std::string name;
		std::filesystem::path pathFromRoot;
		
		FolderNode* parent;
		std::vector<FolderNode*> subNodes;
		std::vector<MaterialInfo> subMaterials;

		bool SubMaterialExistsAlready(std::string& name);
		bool HasMaterials();

		FolderNode* GetSubNodeByName(std::string& name);
		bool IsLeafNode();
		bool IsRoot();
		bool SubnodeExistsAlready(std::string& name);
	};

} // !mnemosy::systems

#endif // !FOLDER_TREE_NODE_H
