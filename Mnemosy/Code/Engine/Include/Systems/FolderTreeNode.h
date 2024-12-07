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
		FolderNode* parent = nullptr; // 8
		LibEntryType type; // 4
		
		uint16_t runtime_ID; // 2
		bool selected = false; // 1
		bool thumbnailLoaded = false; // 1
		unsigned int thumbnailTexure_ID = 0; // 4
		std::string name; // 40


		std::filesystem::path GetPathFromRoot();
	};


	// this struct wastes a lot of padding space
	struct FolderNode {
	public:

		std::vector<LibEntry*> subEntries; 
		std::vector<FolderNode*> subNodes; 

		std::string name;
		std::filesystem::path pathFromRoot;
		
		FolderNode* parent;
		uint16_t runtime_ID;



		
		
		bool HasMaterials();
		void SortLibEntries();

		bool IsLeafNode();
		bool IsRoot();
		
	};

} // !mnemosy::systems

#endif // !FOLDER_TREE_NODE_H
