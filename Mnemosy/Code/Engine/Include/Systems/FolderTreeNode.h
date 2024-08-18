#ifndef FOLDER_TREE_NODE_H
#define FOLDER_TREE_NODE_H


#include <vector>
#include <string>
#include <filesystem>


namespace mnemosy::systems
{
	// Runtime id  is assigned at runtime when material gets created and never changes until the app closes

	struct FolderNode;

	struct MaterialInfo {
	public:
		unsigned int runtime_ID;
		unsigned int thumbnailTexure_ID = 0;
		FolderNode* parent = nullptr;
		std::string name;
		bool selected = false;
		bool thumbnailLoaded = false;
	};


	struct FolderNode {
	public:

		std::vector<FolderNode*> subNodes;
		std::vector<MaterialInfo*> subMaterials;

		std::filesystem::path pathFromRoot;
		std::string name;
		
		FolderNode* parent;
		unsigned int runtime_ID;
		
		bool SubMaterialExistsAlready(std::string& name);
		bool HasMaterials();

		FolderNode* GetSubNodeByName(std::string& name);
		bool IsLeafNode();
		bool IsRoot();
		bool SubnodeExistsAlready(std::string& name);

		std::string MakeStringLowerCase(std::string& str) {


			std::string strLower = str;
			std::transform(strLower.begin(), strLower.end(), strLower.begin(), [](unsigned char c) { return std::tolower(c); });
		
			return strLower;
		}
	};

} // !mnemosy::systems

#endif // !FOLDER_TREE_NODE_H
