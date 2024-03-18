#ifndef FOLDER_TREE_NODE_H
#define FOLDER_TREE_NODE_H


#include <vector>
#include <string>


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
		std::string name;
		unsigned int runtime_ID; // only used for runtime identification.
		std::string pathFromRoot;
		
		FolderNode* parent;
		std::vector<FolderNode*> subNodes;

		std::vector<MaterialInfo> subMaterials;
		//std::vector<std::string> subMaterialNames;
		bool SubMaterialExistsAlready(std::string name);
		bool HasMaterials();


		FolderNode* GetSubNodeByName(std::string name);
		bool IsLeafNode();
		bool IsRoot();
		bool SubnodeExistsAlready(FolderNode* node, std::string name);
	
	};



} // !mnemosy::systems

#endif // !FOLDER_TREE_NODE_H
