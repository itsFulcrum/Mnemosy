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

	struct LibEntry { // 56 bytes wasting 4 bytes padding
	public:
		FolderNode* parent = nullptr; // 8
		LibEntryType type; // 4
		
		uint16_t runtime_ID; // 2
		bool selected = false; // 1
		bool thumbnailLoaded = false; // 1
		unsigned int thumbnailTexure_ID = 0; // 4
		std::string name; // 32


		std::filesystem::path GetPathFromRoot();
	};

	struct FolderNode { // 128 bytes wasting 6 bytes padding
	public:

		std::string name; // 32
		FolderNode* parent; // 8

		// consider if we can get rid of runtime id
		uint16_t runtime_ID; // 2

		std::vector<FolderNode*> subNodes; // 24
		std::vector<LibEntry*> subEntries; // 24


		// we could potentially replace this with a method walking up parents and constructing the pathFromRoot dynamically.
		//std::filesystem::path pathFromRoot; // 32
		


		std::filesystem::path GetPathFromRoot();
		
		
		bool HasMaterials();
		void SortLibEntries();

		bool IsLeafNode();
		bool IsRoot();
		
	};

} // !mnemosy::systems

#endif // !FOLDER_TREE_NODE_H
