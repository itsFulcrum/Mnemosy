#ifndef MATERIAL_LIBRARY_GUI_PANEL_H
#define MATERIAL_LIBRARY_GUI_PANEL_H

#include "Include/Gui/GuiPanel.h"

#include "ImGui/imgui.h"
#include "ImGui/imgui_stdlib.h"


#include <vector>
#include <memory>

namespace mnemosy::systems
{
	struct MaterialInfo;
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
		void DrawMaterialEntries(systems::FolderNode* node);



	private:
		void AddSubfolder(systems::FolderNode* node);
		void RenameFolder(systems::FolderNode* node, std::string newName);
		void DeleteButKeepChildren(systems::FolderNode* node);
		void DeleteHierarchy(systems::FolderNode* node);
		


		void AddMaterial(systems::FolderNode* node);
		void RenameMaterial(systems::FolderNode* node, systems::MaterialInfo* materialInfo, std::string& newName, int positionInVector);
		void DeleteMaterial(systems::FolderNode* node, systems::MaterialInfo* materialInfo, int positionInVector);


		void HandleDeleteHierarchyModal();


	private:
		systems::FolderNode* m_nodeDeleteHierarchy = nullptr;

		ImGuiTreeNodeFlags m_directoryTreeFlags;
		ImGuiTreeNodeFlags m_materialTreeFlags;

		systems::MaterialLibraryRegistry& m_materialRegistry;
		systems::FolderNode* rootNode = nullptr; // pointer lifetime is handled by the engine


		//systems::FolderNode* m_selectedNode = nullptr;
		const char* m_rightClickFolderOptions[5] = {"Add Subfolder", "Add Material", "Delete", "Delete Hierarchy", "Open in FileExplorer"};
		const char* m_rightClickMaterialOptions[2] = { "Delete Selection", "Open in FileExplorer" };
		
		ImGuiInputTextFlags m_textInputFlags = ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_EnterReturnsTrue;
		std::string m_renameMaterialText = "";
		std::string m_renameFolderText = "";
		std::string m_searchInput = "";

		unsigned int m_folderIdToOpenNextFrame = 0;
		bool m_setFolderOpenNextFrame = false;

		bool showDeleteHierarchyModel = false;
		bool m_matDragDropBegin = false;
		MaterialDragDropPayload m_tempStoreMatPayload;


	};

} // !mnemosy::gui

#endif // !MATERIAL_LIBRARY_GUI_PANEL_H
