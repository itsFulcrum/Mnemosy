#ifndef FOLDER_TREE_NODE_H
#define FOLDER_TREE_NODE_H


#include <vector>
#include <string>
#include <filesystem>


namespace mnemosy::systems
{
	enum LibEntryType
	{
		MNSY_ENTRY_TYPE_PBRMAT		= 0,
		MNSY_ENTRY_TYPE_UNLITMAT	= 1,
		MNSY_ENTRY_TYPE_SKYBOX		= 2,
		MNSY_ETRY_TYPE_COUNT
	};


	// Runtime id  is assigned at runtime when material gets created and never changes until the app closes

	struct FolderNode;

	struct LibEntry {
	public:
		LibEntryType type;
		uint16_t runtime_ID;
		unsigned int thumbnailTexure_ID = 0;
		FolderNode* parent = nullptr;
		std::string name;
		bool selected = false;
		bool thumbnailLoaded = false;

		std::filesystem::path GetPathFromRoot();
	};


	struct FolderNode {
	public:

		std::vector<FolderNode*> subNodes;
		std::vector<LibEntry*> subEntries;

		std::filesystem::path pathFromRoot;
		std::string name;
		
		FolderNode* parent;
		unsigned int runtime_ID;
		
		bool HasMaterials();
		void SortLibEntries();

		bool IsLeafNode();
		bool IsRoot();
		
	};

} // !mnemosy::systems

#endif // !FOLDER_TREE_NODE_H
