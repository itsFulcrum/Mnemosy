#ifndef MATERIAL_LIBRARY_GUI_PANEL_H
#define MATERIAL_LIBRARY_GUI_PANEL_H

#include "Include/Gui/GuiPanel.h"

#include "ImGui/imgui.h"
#include "ImGui/imgui_stdlib.h"


#include <vector>
#include <memory>

namespace mnemosy::systems
{
	enum LibEntryType;
	struct LibEntry;
	struct FolderNode;
	class MaterialLibraryRegistry;
}

namespace mnemosy::gui
{
	struct PointerList {
		std::vector<unsigned int> materialListIndexes;
	};

	struct MaterialDragDropPayload {
		systems::FolderNode* sourceNode = nullptr;
		std::shared_ptr<PointerList> matList;
	};


	class MaterialLibraryGuiPanel : public GuiPanel
	{

	public:
		MaterialLibraryGuiPanel();

		virtual void Draw() override;

		void RecursivDrawSubfolders(systems::FolderNode* node);
		void Draw_LibEntries(systems::FolderNode* node);



	private:
		void DrawLibrarySelection();



		void PopupModal_Folder_DeleteHierarchy_Open(systems::FolderNode* folder);
		void PopupModal_Folder_DeleteHierarchy();




	private:
		systems::FolderNode* m_popupModal_folder_deleteHierarchy_ptr = nullptr;

		ImGuiTreeNodeFlags m_directoryTreeFlags;
		ImGuiTreeNodeFlags m_materialTreeFlags;

		systems::MaterialLibraryRegistry& m_materialRegistry;


		//systems::FolderNode* rootNode = nullptr; // pointer lifetime is handled by the engine


		//systems::FolderNode* m_selectedNode = nullptr;
		const char* m_rightClickFolderOptions[7] = {"Add Subfolder", "Add PBR Material", "Add Unlit Texture", "Add Skybox" , "Delete", "Delete Hierarchy", "Open System Folder"};
		const char* m_rightClickMaterialOptions[3] = { "Load", "Delete Selection", "Open System Folder" };
		
		ImGuiInputTextFlags m_textInputFlags = ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_EnterReturnsTrue;
		std::string m_renameMaterialText = "";
		std::string m_renameFolderText = "";
		std::string m_searchInput = "";
		std::string m_renameLibCollectionText = "";

		unsigned int m_folderIdToOpenNextFrame = 0;
		bool m_setFolderOpenNextFrame = false;

		bool m_popupModal_folder_deleteHierarchy_triggered = false;


		bool m_matDragDropBegin = false;
		MaterialDragDropPayload m_tempStoreMatPayload;


	};

} // !mnemosy::gui

#endif // !MATERIAL_LIBRARY_GUI_PANEL_H
