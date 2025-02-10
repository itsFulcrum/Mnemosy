#ifndef GUI_PANELS_COMMON_H
#define GUI_PANELS_COMMON_H


#include "ImGui/imgui.h"

#include <string>

#define TextColor_default	ImVec4(0.68f, 0.68f, 0.68f, 1.00f)
#define TextColor_pbr		ImVec4(0.65f, 0.56f, 0.50f, 1.00f)
#define TextColor_skybox	ImVec4(0.51f, 0.67f, 0.68f, 1.0f)
#define TextColor_unlit		ImVec4(0.51f, 0.60f, 0.51f, 1.00f)


namespace mnemosy::systems {
	struct FolderNode;
	struct LibEntry;
	enum LibEntryType;
}


namespace mnemosy::gui {
	

	struct LibEntry_Payload {
		systems::LibEntry* libEntry = nullptr;
	};

	struct Folder_Payload {
		systems::FolderNode* folderNode = nullptr;
	};

	// these are just common helpers that forward to 'MaterialLibrarySystem' of the engine.

	class GuiProcedures {
	public:

		// folder

		static void folder_create_new(systems::FolderNode* parentNode, bool openFolderAfter);
		
		static void folder_rename(systems::FolderNode* folder, std::string& newName);

		static void folder_delete_move_children_to_parent(systems::FolderNode* folder);

		static void folder_delete_hierarchy(systems::FolderNode* folder);

		// lib entry 

		static void libEntry_create_new(systems::FolderNode* parent, std::string name, systems::LibEntryType entryType,bool openParentFolderAfter);

		static void libEntry_rename(systems::LibEntry* libEntry, std::string& newName);

		static void libEntry_delete(systems::LibEntry* libEntry, unsigned int vectorListIndex);
	};
}

#endif // !GUI_PANELS_COMMON_H
