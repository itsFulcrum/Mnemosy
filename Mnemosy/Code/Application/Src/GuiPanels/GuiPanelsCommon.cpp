#include "Include/GuiPanels/GuiPanelsCommon.h"

#include "Include/MnemosyEngine.h"
#include "Include/Core/Log.h"
#include "Include/Systems/MaterialLibraryRegistry.h"
#include "Include/Systems/FolderTreeNode.h"

namespace mnemosy::gui {


	void GuiProcedures::folder_create_new(systems::FolderNode* parentNode, bool openFolderAfter) {

		std::string newName = "New Folder";
		systems::FolderNode* newFolder = MnemosyEngine::GetInstance().GetMaterialLibraryRegistry().AddNewFolder(parentNode, newName);

		if (openFolderAfter) {
			MnemosyEngine::GetInstance().GetMaterialLibraryRegistry().OpenFolderNode(newFolder);
		}
	}

	void GuiProcedures::folder_rename(systems::FolderNode* folder, std::string& newName) {

		if (newName != folder->name) {

			MnemosyEngine::GetInstance().GetMaterialLibraryRegistry().RenameFolder(folder,newName);
		}
	}

	void GuiProcedures::folder_delete_move_children_to_parent(systems::FolderNode* folder) {

		MnemosyEngine::GetInstance().GetMaterialLibraryRegistry().DeleteAndKeepChildren(folder);
	}

	void GuiProcedures::folder_delete_hierarchy(systems::FolderNode* folder) {

		MnemosyEngine::GetInstance().GetMaterialLibraryRegistry().DeleteFolderHierarchy(folder);
	}

	void GuiProcedures::libEntry_create_new(systems::FolderNode* parent, std::string name, systems::LibEntryType entryType, bool openParentFolderAfter) {

		MnemosyEngine::GetInstance().GetMaterialLibraryRegistry().LibEntry_CreateNew(parent,entryType, name);

		if (openParentFolderAfter) {
			MnemosyEngine::GetInstance().GetMaterialLibraryRegistry().OpenFolderNode(parent);
		}
	}

	void GuiProcedures::libEntry_rename(systems::LibEntry* libEntry, std::string& newName) {

		if (newName != libEntry->name) {
			MnemosyEngine::GetInstance().GetMaterialLibraryRegistry().LibEntry_Rename(libEntry,newName);
		}
	}

	void GuiProcedures::libEntry_delete(systems::LibEntry* libEntry, unsigned int vectorListIndex) {

		MnemosyEngine::GetInstance().GetMaterialLibraryRegistry().LibEntry_Delete(libEntry,static_cast<int>(vectorListIndex));
	}





}
