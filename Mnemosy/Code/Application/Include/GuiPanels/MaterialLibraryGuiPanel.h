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

	class MaterialLibraryGuiPanel : public GuiPanel
	{

	public:
		MaterialLibraryGuiPanel();

		virtual void Draw() override;

		void RecursivDrawSubfolders(systems::FolderNode* node);
		
	private:

		void ChangeNodeName(systems::FolderNode* node, std::string newName);

		ImGuiTreeNodeFlags treeNodeFlags;

		systems::MaterialLibraryRegistry& m_materialRegistry;
		systems::FolderNode* rootNode = nullptr; // pointer lifetime is handled by the engine
	};

} // !mnemosy::gui

#endif // !MATERIAL_LIBRARY_GUI_PANEL_H
