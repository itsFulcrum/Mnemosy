#ifndef FOLDER_TREE_NODE_H
#define FOLDER_TREE_NODE_H


#include <vector>
#include <string>


namespace mnemosy::systems
{
	//struct MaterialInfo
	//{
	//	std::string name;
	//	std::string pathToDataFile;
	//	std::string pathToThumbnail;
	//};

	struct FolderNode {
	
	public:
		std::string name;
		unsigned int runtime_ID; // only used for runtime identification.
		std::string pathFromRoot;
		
		FolderNode* parent;
		std::vector<FolderNode*> subNodes;

		std::vector<std::string> subMaterialNames;
		bool SubMaterialExistsAlready(std::string name);



		FolderNode* GetSubNodeByName(std::string name);
		bool IsLeafNode();
		bool IsRoot();
		bool SubnodeExistsAlready(FolderNode* node, std::string name);
	
	};



} // !mnemosy::systems

#endif // !FOLDER_TREE_NODE_H
