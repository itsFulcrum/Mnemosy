#include "Include/GuiPanels/MaterialLibraryGuiPanel.h"


#include "Include/Core/Log.h"
#include "Include/Application.h"
#include "Include/MnemosyEngine.h"
#include "Include/Core/FileDirectories.h"
#include "Include/Systems/MaterialLibraryRegistry.h"

#include <string>

namespace mnemosy::gui
{
	MaterialLibraryGuiPanel::MaterialLibraryGuiPanel()
		: m_materialRegistry { ENGINE_INSTANCE().GetMaterialLibraryRegistry() }
	{
		panelName = "Material Library";
		rootNode = m_materialRegistry.GetRootFolder();


		treeNodeFlags = ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_SpanAllColumns;

	}
	void MaterialLibraryGuiPanel::Draw()
	{
		if (!showPanel)
			return;

		//MnemosyEngine& engine = ENGINE_INSTANCE();

		ImGui::Begin(panelName.c_str(), &showPanel);

		ImGui::SeparatorText("Material Library");

		if (ImGui::Button("Save All"))
		{
			m_materialRegistry.SaveUserDirectoriesData();

		}


		RecursivDrawSubfolders(rootNode);


		ImGui::End();

	}
	void MaterialLibraryGuiPanel::RecursivDrawSubfolders(systems::FolderNode* node) {
		
		if (node == nullptr)
			return;


		// Makes leaf Folder not have expanding behavior
		ImGuiTreeNodeFlags thisNodeFlags = treeNodeFlags;
		if (node->IsLeafNode()) {
			thisNodeFlags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_OpenOnDoubleClick;
		}

		if (ImGui::TreeNodeEx(node->name.c_str(), thisNodeFlags)) {
			
			ImGui::SameLine();

			// right click on open folder to rename
			if (ImGui::BeginPopupContextItem())
			{
				static std::string newName = "New Folder";
				// == Rename Folder
				if (ImGui::Button("Rename"))
				{
					ChangeNodeName(node, newName);
				}
				ImGui::SameLine();
				ImGui::InputText("##edit", &newName);

				// == Subfolder
				if (ImGui::Button("Add Subfolder"))
				{
					// making Sure that name is a unique subname
					int suffix = 1;
					std::string folderName = newName;
					std::string newFolderName = folderName;// +"_" + std::to_string(suffix);
					while (node->SubnodeExistsAlready(node, newFolderName)) {
						newFolderName = folderName + "_" + std::to_string(suffix);
						suffix++;
					}

					node->subNodes.push_back(m_materialRegistry.CreateFolderNode(node, newFolderName));
				}
				ImGui::SameLine();
				
				// == Add Material Entry
				if (ImGui::Button("Add Material")) {
					
				}

				if (ImGui::Button("Delete Hierarchy")) {

					m_materialRegistry.DeleteFolderHierarchy(node);
				}


				ImGui::EndPopup();
			}

			ImGui::Spacing();
			
			if (ImGui::BeginDragDropSource())
			{
				ImGui::SetDragDropPayload("_TREENODE", NULL, 0);
				ImGui::Text("This is a drag and drop source");
				ImGui::EndDragDropSource();
			}

			if (!node->IsLeafNode()) {

				for (int i = 0; i < node->subNodes.size(); i++) {

					RecursivDrawSubfolders(node->subNodes[i]);
				}

			}
			




			ImGui::TreePop();
		}

		
	}
	void MaterialLibraryGuiPanel::ChangeNodeName(systems::FolderNode* node, std::string newName)
	{
		if (node->name == "Root" || node->parent == nullptr)
		{
			MNEMOSY_WARN("You cannot change the name of the root directory");
			return;

		}

		// makes sure that no node of its parent is called the same
		// this allows to rename a folder to have the same name as one of its childnodes
		int suffix = 1;
		std::string folderName = newName;
		std::string finalName = folderName;
		while (node->parent->SubnodeExistsAlready(node->parent, finalName)) {
			finalName = folderName + "_" + std::to_string(suffix);
			suffix++;
		}

		// maybe call registry to update or somehting
		node->name = finalName;

	}
} // !mnemosy::gui