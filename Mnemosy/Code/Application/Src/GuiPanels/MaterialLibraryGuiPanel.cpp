#include "Include/GuiPanels/MaterialLibraryGuiPanel.h"


#include "Include/Core/Log.h"
#include "Include/Application.h"
#include "Include/MnemosyEngine.h"
#include "Include/Core/FileDirectories.h"
#include "Include/Systems/MaterialLibraryRegistry.h"
#include "Include/Systems/FolderTreeNode.h"

#include <string>

namespace mnemosy::gui
{
	MaterialLibraryGuiPanel::MaterialLibraryGuiPanel()
		: m_materialRegistry { ENGINE_INSTANCE().GetMaterialLibraryRegistry() }
	{
		panelName = "Material Library";
		rootNode = m_materialRegistry.GetRootFolder();
		m_selectedNode = rootNode;

		m_directoryTreeFlags = ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_SpanAllColumns;
		m_materialTreeFlags = ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_SpanAllColumns | ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_Bullet;

	}
	void MaterialLibraryGuiPanel::Draw() {
		
		if (!showPanel)
			return;

		ImGui::Begin(panelName.c_str(), &showPanel);
		{
			ImGui::BeginChild("Library", ImVec2(0, 350), ImGuiChildFlags_Border | ImGuiChildFlags_ResizeY);
			{

				ImGui::SeparatorText("Material Library");

				if (ImGui::Button("Save All")) {
					m_materialRegistry.SaveUserDirectoriesData();
				}

				RecursivDrawSubfolders(rootNode);
				HandleDeleteHierarchyModal();
			}
			ImGui::EndChild();



			ImGui::BeginChild("CurrentDirectory", ImVec2(0, 0), ImGuiChildFlags_Border );
			{

			
				std::string directoryText = "Directory: " + m_selectedNode->name;
				ImGui::SeparatorText(directoryText.c_str());
			
				ImGuiStyle& style = ImGui::GetStyle();
				float window_visible_x2 = ImGui::GetWindowPos().x + ImGui::GetWindowContentRegionMax().x;
				
				ImVec2 button_size(100, 100);
				int materialCount = m_selectedNode->subMaterialNames.size();
				for (int i = 0; i < materialCount; i++) {

					if (ImGui::Button(m_selectedNode->subMaterialNames[i].c_str(), button_size)) {

					}


					float last_button_x2 = ImGui::GetItemRectMax().x;
					float next_button_x2 = last_button_x2 + style.ItemSpacing.x + button_size.x;
					if (i + 1 < materialCount && next_button_x2 < window_visible_x2)
						ImGui::SameLine();
				}

			}
			ImGui::EndChild();


		}
		ImGui::End();

	}

	void MaterialLibraryGuiPanel::RecursivDrawSubfolders(systems::FolderNode* node) {
		
		if (node == nullptr)
			return;

		// Makes leaf Folder not have expanding behavior
		//ImGuiTreeNodeFlags thisNodeFlags = treeNodeFlags;
		//if (node->IsLeafNode() && node->subMaterialNames.empty()) {
		//	thisNodeFlags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_OpenOnDoubleClick;
		//}

		ImGuiTreeNodeFlags node_flags = m_directoryTreeFlags;

		if (m_selectedNode->runtime_ID == node->runtime_ID)
			node_flags |= ImGuiTreeNodeFlags_Selected;


		bool nodeOpen = ImGui::TreeNodeEx(node->name.c_str(), node_flags);
			
		if (ImGui::IsItemClicked() && !nodeOpen) {
			m_selectedNode = node;
		}

		if (nodeOpen) {

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
							AddMaterial(node,newName);
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
			
			if (!node->IsRoot())  {  // dont allow root to be a drag and drop source

				if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) {

					unsigned int sourceNodeID = node->runtime_ID;
					ImGui::SetDragDropPayload("DragPayload_ID", &sourceNodeID, sizeof(unsigned int));
					ImGui::EndDragDropSource();
				}
			}
			
			if (ImGui::BeginDragDropTarget()) {

				// Directory DragDrop Target For moving a directory underneith
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DragPayload_ID")) {
					//MNEMOSY_TRACE("Directory dragdrop target");

					IM_ASSERT(payload->DataSize == sizeof(unsigned int));
					unsigned int sourceNodeID = *(const unsigned int*)payload->Data;
					
					systems::FolderNode* sourceNode = m_materialRegistry.RecursivGetNodeByRuntimeID(m_materialRegistry.GetRootFolder(),sourceNodeID);

					if (sourceNode != nullptr) { // just in case. This should never happen.

						if (sourceNode->parent->name != node->name) { // dont copy if it is already in that directory
							m_materialRegistry.MoveDirectory(sourceNode, node);
						}
						else {
							MNEMOSY_WARN("Dirctory is already inside this directory");
						}
					} 
					else {
						MNEMOSY_ERROR("Could not move directory, because it didn't find the source node by its id. \nThis should not happen and is most likely a bug");
					}

				}

				// Material dragDrop target for moving a material underneith
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DragDropMaterialPayload")) {
					//MNEMOSY_TRACE("Material dragdrop target");

					IM_ASSERT(payload->DataSize == sizeof(MaterialDragDropPayload));
					MaterialDragDropPayload materialPayload = *(const MaterialDragDropPayload*)payload->Data;

					systems::FolderNode* sourceNode = m_materialRegistry.RecursivGetNodeByRuntimeID(m_materialRegistry.GetRootFolder(), materialPayload.nodeRuntimeID);
					
					std::string matName = sourceNode->subMaterialNames[materialPayload.nodeMaterialIndex];
					MNEMOSY_TRACE("Material payload: SourceNode: {}, Material {}", sourceNode->name,matName);
					if (sourceNode != nullptr) { // just in case. This should never happen.


						if (node->SubMaterialExistsAlready(matName)) {
							
							MNEMOSY_WARN("This Material already exists in the target folder");
						}
						else {
							m_materialRegistry.MoveMaterial(sourceNode,node,matName);
						}

						
					}
					else {
						MNEMOSY_ERROR("Could not move material, because it didn't find the source node by its id. \nThis should not happen and is most likely a bug");
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
			
			// === Draw Material Entries
			DrawMaterialEntries(node);

		
			ImGui::TreePop();
		
		}
		
	}

	void MaterialLibraryGuiPanel::DrawMaterialEntries(systems::FolderNode* node) {

		if (node->subMaterialNames.empty())
			return;

		for (int i = 0; i < node->subMaterialNames.size(); i++) {
				

			if (ImGui::TreeNodeEx(node->subMaterialNames[i].c_str(), m_materialTreeFlags)) {


				// === Right Click Options
				const char* materialOptions[] = { "Rename", "Delete" };
				// right click on open folder to rename
				if (ImGui::BeginPopupContextItem()) {

					static std::string newMaterialName = "New Material";

					ImGui::Text("Name: ");
					ImGui::SameLine();
					ImGui::InputText("##edit", &newMaterialName);

					for (int b = 0; b < IM_ARRAYSIZE(materialOptions); b++) {

						if (ImGui::Selectable(materialOptions[b])) {
							if (b == 0) { // rename
								MNEMOSY_TRACE("Selection rename: material: {}, new Name: {}", node->subMaterialNames[i], newMaterialName);
								//materialName = newMaterialName;
								ChangeMaterialName(node, node->subMaterialNames[i], newMaterialName,i);
							}
							else if (b == 1) { // delete
								MNEMOSY_TRACE("Selection delete");
								// TODO make PopupModal for delete 
								DeleteMaterial(node, node->subMaterialNames[i], i);
							}

						}
					} // end options loop
					ImGui::EndPopup();
				} // End Popup Context Item


				if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) {

					MaterialDragDropPayload materialPayload = MaterialDragDropPayload(node->runtime_ID,i);
					//unsigned int sourceNodeID = node->runtime_ID;
					ImGui::SetDragDropPayload("DragDropMaterialPayload", &materialPayload, sizeof(MaterialDragDropPayload));

					ImGui::EndDragDropSource();
				}


				ImGui::TreePop();
			}




		} // end loop through sub materials
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
		m_selectedNode = rootNode;
		m_materialRegistry.DeleteFolderHierarchy(node);
	}

	void MaterialLibraryGuiPanel::ChangeNodeName(systems::FolderNode* node, std::string newName) {

		if (node->IsRoot()) {
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
		if (node->parent->IsRoot()) {
			node->pathFromRoot = finalName;
		}
		else {
			node->pathFromRoot = node->parent->pathFromRoot + "/" + finalName;
		}

		m_materialRegistry.RenameDirectory(node, oldPathFromRoot);

	}
	
	
	void MaterialLibraryGuiPanel::AddMaterial(systems::FolderNode* node,std::string name) {

		m_materialRegistry.CreateNewMaterial(node, "New Material");
	}
	void MaterialLibraryGuiPanel::ChangeMaterialName(systems::FolderNode* node, std::string& materialName, std::string& newName, int positionInVector) {

		m_materialRegistry.ChangeMaterialName(node, materialName, newName,positionInVector);

	}
	void MaterialLibraryGuiPanel::DeleteMaterial(systems::FolderNode* node, std::string& materialName, int positionInVector) {
	
		m_materialRegistry.DeleteMaterial(node, materialName, positionInVector);
	}
} // !mnemosy::gui