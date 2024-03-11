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
	void MaterialLibraryGuiPanel::Draw() {
		
		if (!showPanel)
			return;

		ImGui::Begin(panelName.c_str(), &showPanel);
		{
			ImGui::SeparatorText("Material Library");

			if (ImGui::Button("Save All")) {
				m_materialRegistry.SaveUserDirectoriesData();
			}

		
			RecursivDrawSubfolders(rootNode);
		
			HandleDeleteHierarchyModal();

		}
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
			
			// === Right Click Options
			const char* options[] = { "Rename", "Add Subfolder", "Add Material", "Delete Hierarchy"};
			// right click on open folder to rename
			if (ImGui::BeginPopupContextItem()) {
				
				static std::string newName = "New Folder";

				ImGui::Text("Name: ");
				ImGui::SameLine();
				ImGui::InputText("##edit", &newName);

				for (int i = 0; i < IM_ARRAYSIZE(options); i++) {

					if (ImGui::Selectable(options[i])) {

						if (i == 0)  { // rename
							// need to open another popup probably
							ChangeNodeName(node, newName);
						}
						else if (i == 1) { // Add Subfolder

							AddSubfolder(node, newName);
						}
						else if (i == 2) { // Add material 

						}
						else if (i == 3) { // delete hierarchy
						
							m_nodeDeleteHierarchy = node;
							showDeleteHierarchyModel = true;
						}

					}
				} // end options loop
				ImGui::EndPopup();
			} // End Popup Context Item

			ImGui::Spacing();

			// == DRAG AND DROP
			
			if (node->name != "Root")  {  // dont allow root to be a drag and drop source

				if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) {

					unsigned int sourceNodeID = node->runtime_ID;
					ImGui::SetDragDropPayload("DragPayload_ID", &sourceNodeID, sizeof(unsigned int));
					ImGui::EndDragDropSource();
				}
			}

			if (ImGui::BeginDragDropTarget()) {

				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DragPayload_ID")) {

					IM_ASSERT(payload->DataSize == sizeof(unsigned int));
					unsigned int sourceNodeID = *(const unsigned int*)payload->Data;
					
					systems::FolderNode* sourceNode = m_materialRegistry.RecursivGetNodeByRuntimeID(m_materialRegistry.GetRootFolder(),sourceNodeID);

					if (sourceNode != nullptr) { // just in case. This should never happen.

						if (sourceNode->parent->name != node->name) { // dont copy if it is already in that directory
							m_materialRegistry.MoveDirectory(sourceNode, node);
						}
						else {
							MNEMOSY_WARN("Moving a dirctory into the directory it is already in, is pointless bro..");
						}
					} 
					else {
						MNEMOSY_ERROR("Could not move directory, because it didn't find the source node by its id. \nThis should not happen and is most likely a bug");
					}

				}
				ImGui::EndDragDropTarget();
			}

			// === Recusivly draw Sub directories

			if (!node->IsLeafNode()) {
				for (int i = 0; i < node->subNodes.size(); i++) {
					RecursivDrawSubfolders(node->subNodes[i]);
				}
			}
			


			ImGui::TreePop();
		}
	}

	void MaterialLibraryGuiPanel::AddSubfolder(systems::FolderNode* node, std::string name) {

		int suffix = 1;
		std::string folderName = name;
		std::string newFolderName = folderName;// +"_" + std::to_string(suffix);
		while (node->SubnodeExistsAlready(node, newFolderName)) {
			newFolderName = folderName + "_" + std::to_string(suffix);
			suffix++;
		}

		node->subNodes.push_back(m_materialRegistry.CreateFolderNode(node, newFolderName));
	}

	void MaterialLibraryGuiPanel::HandleDeleteHierarchyModal() {

		static bool popModal = false;
		if (showDeleteHierarchyModel) {

			popModal = true;
			showDeleteHierarchyModel = false; // to make sure its only called once
			ImGui::OpenPopup("Delete Hierarchy");
		}

		if (ImGui::BeginPopupModal("Delete Hierarchy", &popModal, ImGuiWindowFlags_AlwaysAutoResize)) {

			ImGui::Text("Are you sure? \nAll files underneith will be deleted permanently!");

			if (ImGui::Button("Cancel", ImVec2(120, 0))) {
				m_nodeDeleteHierarchy = nullptr;
				popModal = false;
				ImGui::CloseCurrentPopup();
			}

			ImGui::SameLine();

			if (ImGui::Button("Delete..")) {

				if(m_nodeDeleteHierarchy)
					DeleteHierarchy(m_nodeDeleteHierarchy);
				
				
				popModal = false;
				m_nodeDeleteHierarchy = nullptr;
				ImGui::CloseCurrentPopup();
			}

			ImGui::EndPopup();
		}


	}

	void MaterialLibraryGuiPanel::DeleteHierarchy(systems::FolderNode* node) {

		// make popup modal 
		m_materialRegistry.DeleteFolderHierarchy(node);
	}
	void MaterialLibraryGuiPanel::ChangeNodeName(systems::FolderNode* node, std::string newName) {

		if (node->name == "Root" || node->parent == nullptr) {

			MNEMOSY_WARN("You cannot change the name of the root directory");
			return;
		}

		std::string oldPathFromRoot = node->pathFromRoot;
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

		// updating path from root so we can rename the file.  !! we are not yet updateing all subsequent pathsFromRoot for sub directories .. it'll only happen when saving atm
		if (node->parent->name == "Root") { // if parent is root

			node->pathFromRoot = finalName;
		}
		else {
			node->pathFromRoot = node->parent->pathFromRoot + "/" + finalName;
		}


		m_materialRegistry.RenameDirectory(node, oldPathFromRoot);
	}
} // !mnemosy::gui