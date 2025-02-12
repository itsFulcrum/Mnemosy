#ifndef GUI_PANELS_COMMON_H
#define GUI_PANELS_COMMON_H


#include "ImGui/imgui.h"

#include <string>

#define TextColor_default	ImVec4(0.68f, 0.68f, 0.68f, 1.00f)
#define TextColor_pbr		ImVec4(0.65f, 0.56f, 0.50f, 1.00f)
#define TextColor_skybox	ImVec4(0.51f, 0.67f, 0.68f, 1.0f)
#define TextColor_unlit		ImVec4(0.51f, 0.60f, 0.51f, 1.00f)



#define Gui_Txt_Color_Trace ImVec4( 0.850f , 0.850f , 0.850f , 1.0f) // white
#define Gui_Txt_Color_Debug ImVec4( 0.149f , 0.661f , 0.866f , 1.0f) // blue
#define Gui_Txt_Color_Info  ImVec4( 0.203f , 0.741f , 0.243f , 1.0f) // green
#define Gui_Txt_Color_Warn  ImVec4( 0.866f , 0.756f , 0.113f , 1.0f) // yellow
#define Gui_Txt_Color_Error ImVec4( 0.874f , 0.200f , 0.200f , 1.0f) // red
#define Gui_Txt_Color_Fatal ImVec4( 1.000f , 0.000f , 0.000f , 1.0f) // red 


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

		static void SamelineTooltip(const char* msg);
	};
}

#endif // !GUI_PANELS_COMMON_H
