#ifndef MATERIAL_LIBRARY_GUI_PANEL_H
#define MATERIAL_LIBRARY_GUI_PANEL_H

#include "Include/Gui/GuiPanel.h"

#include "External/ImGui/imgui.h"
#include "External/ImGui/imgui_stdlib.h"

namespace mnemosy::systems
{
	struct FolderNode;
	class MaterialLibraryRegistry;
}

namespace mnemosy::gui
{
	struct MaterialDragDropPayload {
		unsigned int nodeRuntimeID;
		unsigned int nodeMaterialIndex;

		MaterialDragDropPayload(unsigned int nodeID, unsigned int materialID) {
			nodeRuntimeID = nodeID;
			nodeMaterialIndex = materialID;
		}
	};

	class MaterialLibraryGuiPanel : public GuiPanel
	{

	public:
		MaterialLibraryGuiPanel();

		virtual void Draw() override;

		void RecursivDrawSubfolders(systems::FolderNode* node);
		void DrawMaterialEntries(systems::FolderNode* node);
	private:
		void AddSubfolder(systems::FolderNode* node,std::string name);
		void ChangeNodeName(systems::FolderNode* node, std::string newName);
		
		void AddMaterial(systems::FolderNode* node,std::string name);
		void ChangeMaterialName(systems::FolderNode* node,std::string& materialName, std::string& newName, int positionInVector);
		void DeleteMaterial(systems::FolderNode* node, std::string& materialName, int positionInVector);


		void HandleDeleteHierarchyModal();
		void DeleteHierarchy(systems::FolderNode* node);
		systems::FolderNode* m_nodeDeleteHierarchy = nullptr;

		ImGuiTreeNodeFlags m_directoryTreeFlags;
		ImGuiTreeNodeFlags m_materialTreeFlags;

		systems::MaterialLibraryRegistry& m_materialRegistry;
		systems::FolderNode* rootNode = nullptr; // pointer lifetime is handled by the engine


		systems::FolderNode* m_selectedNode = nullptr;

		bool showDeleteHierarchyModel = false;
	};

} // !mnemosy::gui

#endif // !MATERIAL_LIBRARY_GUI_PANEL_H
