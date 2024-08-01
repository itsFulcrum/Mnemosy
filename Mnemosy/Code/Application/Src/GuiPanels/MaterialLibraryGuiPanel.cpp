#include "Include/GuiPanels/MaterialLibraryGuiPanel.h"


#include "Include/Core/Log.h"
#include "Include/Application.h"
#include "Include/ApplicationConfig.h"

#include "Include/MnemosyEngine.h"
#include "Include/Graphics/Renderer.h"
#include "Include/Core/FileDirectories.h"
#include "Include/Systems/MaterialLibraryRegistry.h"
#include "Include/Systems/FolderTreeNode.h"
#include "Include/Systems/ThumbnailManager.h"

#ifdef MNEMOSY_PLATFORM_WINDOWS
#include "Include/Core/Utils/PlatfromUtils_Windows.h"
#endif // MNEMOSY_PLATFORM_WINDOWS


#include <string>

namespace mnemosy::gui
{
	MaterialLibraryGuiPanel::MaterialLibraryGuiPanel()
		: m_materialRegistry { MnemosyEngine::GetInstance().GetMaterialLibraryRegistry()}
	{
		panelName = "Material Library";
		panelType = MNSY_GUI_PANEL_MATERIAL_LIBRARY;

		rootNode = m_materialRegistry.GetRootFolder();
		//m_selectedNode = rootNode;
		m_materialRegistry.OpenFolderNode(rootNode);


		m_directoryTreeFlags = ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_SpanAllColumns | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_OpenOnArrow;
		m_materialTreeFlags = ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_SpanAllColumns | ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_Bullet;

	}

	void MaterialLibraryGuiPanel::Draw() {
		
		if (!showPanel)
			return;

		ImGui::Begin(panelName, &showPanel);
		{
			ImGui::BeginChild("Library", ImVec2(0, 350), ImGuiChildFlags_Border | ImGuiChildFlags_ResizeY);
			{

				ImGui::SeparatorText("Material Library");

				RecursivDrawSubfolders(rootNode);
				HandleDeleteHierarchyModal();
			}
			ImGui::EndChild();


			// Draw image buttns of active folder
			ImGui::BeginChild("CurrentDirectory", ImVec2(0, 0), ImGuiChildFlags_Border );
			{

				if (m_materialRegistry.GetSelectedNode() == nullptr) {
					MNEMOSY_TRACE("Selected node is not nullptr");
				}
			
				std::string directoryText = "Directory: " + m_materialRegistry.GetSelectedNode()->name;
				ImGui::SeparatorText(directoryText.c_str());

				static float buttonSize = 128.0f;
				ImGui::SliderFloat("Icon Size",&buttonSize,32.0f,350.0f,"%.0f");
				ImGui::Spacing();

				if (m_materialRegistry.GetSelectedNode()->HasMaterials()) {
					// to do kinda messy tbh registry could store it too
					fs::path lib = MnemosyEngine::GetInstance().GetFileDirectories().GetLibraryDirectoryPath();
					fs::path activeFolderPath =  lib / fs::path( m_materialRegistry.GetSelectedNode()->pathFromRoot);

					MnemosyEngine::GetInstance().GetThumbnailManager().LoadThumbnailsOfActiveFolder(m_materialRegistry.GetSelectedNode(),activeFolderPath);



					systems::FolderNode* selectedNode = m_materialRegistry.GetSelectedNode();

					ImGuiStyle& style = ImGui::GetStyle();
					float window_visible_x2 = ImGui::GetWindowPos().x + ImGui::GetWindowContentRegionMax().x;
					
					ImVec2 button_size(buttonSize, buttonSize);
					int materialCount = selectedNode->subMaterials.size();
					for (int i = 0; i < materialCount; i++) {

						//FIXME: BeginGroup / EndGroup works good but still issue with longer material names causing alignment issues
						ImGui::BeginGroup();
						std::string& matName = selectedNode->subMaterials[i].name;


						//ImTextureID texID = (void*)selectedNode->subMaterials[i].thumbnailTexure_ID;

						//delete texID;
						//MNEMOSY_DEBUG("Texture ID:{}", selectedNode->subMaterials[i].thumbnailTexure_ID);
						

						bool pressed = ImGui::ImageButton((void*)(selectedNode->subMaterials[i].thumbnailTexure_ID), button_size, ImVec2(0, 1), ImVec2(1, 0));
						

						if (pressed) {

							// check if its already the active material
							if (m_materialRegistry.GetActiveMaterialID() != selectedNode->subMaterials[i].runtime_ID) {
								
								fs::path matDir = fs::path(selectedNode->pathFromRoot) / fs::path(matName);
								m_materialRegistry.LoadActiveMaterialFromFile_Multithreaded(matDir,selectedNode->subMaterials[i], selectedNode);
							}
						}


						// Calculate size of the name and shorten it if its longer then the button size
						std::string matNameString = matName;

						while (ImGui::CalcTextSize(matNameString.c_str()).x > (buttonSize - 5.0f )) {
							matNameString.pop_back();
						};

						if (matNameString != matName) {
							matNameString += "..";
						}



#ifdef mnemosy_gui_showDebugInfo
						std::string nameWithDebugInfo = matName + " -GLTexID: " + std::to_string(selectedNode->subMaterials[i].thumbnailTexure_ID);
						ImGui::Text(nameWithDebugInfo.c_str());
#else						
						ImGui::Text(matNameString.c_str());
#endif // mnemosy_gui_showDebugInfo




						ImGui::EndGroup();

						float last_button_x2 = ImGui::GetItemRectMax().x;
						float next_button_x2 = last_button_x2 + style.ItemSpacing.x + button_size.x;
						if (i + 1 < materialCount && next_button_x2 < window_visible_x2)
							ImGui::SameLine();
					}
				}

			}
			ImGui::EndChild();


		}
		ImGui::End();

	}

	void MaterialLibraryGuiPanel::RecursivDrawSubfolders(systems::FolderNode* node) {
		
		if (node == nullptr)
			return;

		
		// prepare node flags
		ImGuiTreeNodeFlags node_flags = m_directoryTreeFlags;

		if (node->IsRoot()) {
			node_flags |= ImGuiTreeNodeFlags_DefaultOpen;
		}


		if (m_setFolderOpenNextFrame ) {

			if (node->runtime_ID == m_folderIdToOpenNextFrame) {
				//m_materialRegistry.OpenFolderNode(node);
				ImGui::SetNextItemOpen(true);
				m_setFolderOpenNextFrame = false;

			}
		}

		if (m_materialRegistry.GetSelectedNode()->runtime_ID == node->runtime_ID) {
			node_flags |= ImGuiTreeNodeFlags_Selected;
		}

		bool nodeOpen = ImGui::TreeNodeEx(node->name.c_str(), node_flags);
			
		
		if (ImGui::IsItemClicked()) {

			m_materialRegistry.OpenFolderNode(node);
		}

		ImGui::SameLine();
			
		// === Right Click Options
		if (ImGui::BeginPopupContextItem()) {
				
			bool isRoot = node->IsRoot();
			if (!isRoot) {
				m_renameFolderText = node->name;

				ImGui::Text("Rename: ");
				ImGui::SameLine();

				bool renamed = ImGui::InputText("##RenameFolderInputField", &m_renameFolderText, m_textInputFlags);
				if (renamed) {

					RenameFolder(node, m_renameFolderText);
				}
			}



			for (int i = 0; i < IM_ARRAYSIZE(m_rightClickFolderOptions); i++) {

				if (ImGui::Selectable(m_rightClickFolderOptions[i])) {

					
					if (i == 0) { // Add Subfolder

						AddSubfolder(node);
					}
					else if (i == 1) { // Add material 
						AddMaterial(node);
					}
					else if (i == 2) { // delete but keep children
						DeleteButKeepChildren(node);
					}
					else if (i == 3) { // delete hierarchy
						
						m_nodeDeleteHierarchy = node;
						showDeleteHierarchyModel = true;
					}
					else if (i == 4) { // open in explorer
						
						fs::path pathToFolder = m_materialRegistry.GetFolderPath(node);
						mnemosy::core::FileDialogs::OpenFolderAt(pathToFolder.generic_string().c_str());
					}

				}

			} // end options loop
			ImGui::EndPopup();
		} // End Popup Right Click Options

		ImGui::Spacing();

		// == DRAG AND DROP
		{

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
					
						systems::FolderNode* sourceNode = m_materialRegistry.GetFolderByID(m_materialRegistry.GetRootFolder(),sourceNodeID);

						if (sourceNode != nullptr) { // just in case. This should never happen.

							if (sourceNode->parent->name != node->name) { // dont copy if it is already in that directory
								m_materialRegistry.MoveFolder(sourceNode, node);
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

						m_matDragDropBegin = false;

						IM_ASSERT(payload->DataSize == sizeof(MaterialDragDropPayload));
						MaterialDragDropPayload materialPayload = *(const MaterialDragDropPayload*)payload->Data;

						// i think we can just pass a pointer no?
						systems::FolderNode* sourceNode = materialPayload.sourceNode;// m_materialRegistry.GetFolderByID(m_materialRegistry.GetRootFolder(), materialPayload.nodeRuntimeID);

						MNEMOSY_ASSERT(sourceNode != nullptr, "A nullpointer folder node should never be passed into a drag and drop payload");

						//MNEMOSY_TRACE("Material Payload  matSize: {}, source node sub material: {}", materialPayload.matList.get()->materialListIndexes.size(), sourceNode->subMaterials.size());


						// need to make a copy of node.submaterials because it changed during the loop
						std::vector<systems::MaterialInfo> subMatsCopy = sourceNode->subMaterials;

						for (unsigned int b = 0; b < materialPayload.matList->materialListIndexes.size(); b++) {

							unsigned int index = materialPayload.matList->materialListIndexes[b];
							//MNEMOSY_TRACE("Material index: {}", index);
							systems::MaterialInfo matInfoTemp = subMatsCopy[index];
							m_materialRegistry.MoveMaterial(sourceNode, node, matInfoTemp);
						}
						
						subMatsCopy.clear();
					}


					ImGui::EndDragDropTarget();
				}
		
		} // Drag and drop

		if (nodeOpen) {
			

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

		if (node->subMaterials.empty())
			return;

		for (size_t i = 0; i < node->subMaterials.size(); i++) {
				
#ifdef mnemosy_gui_showDebugInfo
			std::string materialText = node->subMaterials[i].name + " -MatID: " + std::to_string(node->subMaterials[i].runtime_ID);
#else
			std::string materialText = node->subMaterials[i].name;
#endif // !mnemosy_gui_showDebugInfo


			//bool matIsOpen = ImGui::TreeNodeEx(materialText.c_str(), m_materialTreeFlags);
			bool matIsOpen = true;


			bool selected = ImGui::Selectable(materialText.c_str(), node->subMaterials[i].selected);


			if (ImGui::IsItemClicked()) {
			
				// we need this here to reset it if user dragged but it didnt arrive
				m_matDragDropBegin = false;
			}

			if (selected) {

				if (!ImGui::GetIO().KeyCtrl) {
					// clear selection 
					for (size_t a = 0; a < node->subMaterials.size(); a++) {

						node->subMaterials[a].selected = false;
					}
				}

				node->subMaterials[i].selected = true;	

			}


			if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) {

				if(!m_matDragDropBegin){
					
					m_matDragDropBegin = true;

					// current mat will be selected
					node->subMaterials[i].selected = true;


					MaterialDragDropPayload matPayload = MaterialDragDropPayload();
					matPayload.sourceNode = node;
					matPayload.matList = std::make_shared<PointerList>();


					for (unsigned int a = 0; a < node->subMaterials.size(); a++) {

						if (node->subMaterials[a].selected) {
					
							matPayload.matList.get()->materialListIndexes.push_back(a);

							//MNEMOSY_TRACE("PushBack index {}", matPayload.matList.get()->materialListIndexes.back());
						}
					}
					
					// store it temporarly so that the shared pointer doesn't go out of scope and persists
					m_tempStoreMatPayload = matPayload;

					ImGui::SetDragDropPayload("DragDropMaterialPayload", &matPayload, sizeof(MaterialDragDropPayload));

				}

				// old way with just one draggable material
				//MaterialDragDropPayload materialPayload = MaterialDragDropPayload(node->runtime_ID, i);
				//ImGui::SetDragDropPayload("DragDropMaterialPayload", &materialPayload, sizeof(MaterialDragDropPayload));MaterialDragDropPayload materialPayload = MaterialDragDropPayload(node->runtime_ID, i);

				

				ImGui::EndDragDropSource();
			}


			if (matIsOpen) {

#ifdef mnemosy_gui_showDebugInfo
				if (ImGui::IsItemClicked()) {
					MNEMOSY_DEBUG("Clicked Material: {}, MatID: {}", node->subMaterials[i].name, node->subMaterials[i].runtime_ID);
				}
#endif // mnemosy_gui_showDebugInfo

				// === Right Click Options
				// right click on open folder to open options
				if (ImGui::BeginPopupContextItem()) {

					m_renameMaterialText = node->subMaterials[i].name;


					ImGui::Text("Rename: ");
					ImGui::SameLine();

					bool renamed = ImGui::InputText("##RenameMaterialInputField", &m_renameMaterialText,m_textInputFlags);
					if (renamed) {
						RenameMaterial(node, node->subMaterials[i], m_renameMaterialText, i);
					}

					for (size_t option = 0; option < IM_ARRAYSIZE(m_rightClickMaterialOptions); option++) {

						if (ImGui::Selectable(m_rightClickMaterialOptions[option])) {
							
							if (option == 0) { // delete
								

								std::vector<systems::MaterialInfo> subMatsCopy = node->subMaterials;

								for (size_t a = 0; a < subMatsCopy.size(); a++) {


									if (subMatsCopy[a].selected) {

										int posInList = -1;
										// find posiiton in the original list which is changing as we delete materials
										int runtimeID = subMatsCopy[a].runtime_ID;
										for (size_t b = 0; b < node->subMaterials.size(); b++) {

											if (runtimeID == node->subMaterials[b].runtime_ID) {

												posInList = b;
											}

										}

										MNEMOSY_ASSERT(posInList > -1, "Should always find the correct list entry");

										DeleteMaterial(node, subMatsCopy[a], posInList);

									}

								}

								subMatsCopy.clear();
							} 
							
							else if (option == 1) { // open in FileExplorer

								fs::path pathToMaterialFolder = m_materialRegistry.GetMaterialPath(node, node->subMaterials[i]);
								mnemosy::core::FileDialogs::OpenFolderAt(pathToMaterialFolder.generic_string().c_str());
							}
						}

					} // end options loop
					ImGui::EndPopup();
				} // End Popup Context Item


		


				//ImGui::TreePop();
			}




		} // end loop through sub materials
	}




	void MaterialLibraryGuiPanel::AddSubfolder(systems::FolderNode* node) {

		std::string newName = "New Folder";
		systems::FolderNode* newFolder = m_materialRegistry.AddNewFolder(node, newName);

		m_materialRegistry.OpenFolderNode(newFolder);

		m_setFolderOpenNextFrame = true;
		m_folderIdToOpenNextFrame = node->runtime_ID;
		
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

	void MaterialLibraryGuiPanel::DeleteButKeepChildren(systems::FolderNode* node) {

		m_materialRegistry.DeleteAndKeepChildren(node);
	}

	void MaterialLibraryGuiPanel::DeleteHierarchy(systems::FolderNode* node) {
		m_materialRegistry.DeleteFolderHierarchy(node);
	}

	void MaterialLibraryGuiPanel::RenameFolder(systems::FolderNode* node, std::string newName) {

		if (newName != node->name) {
			m_materialRegistry.RenameFolder(node, newName);
		}
	}
	
	
	void MaterialLibraryGuiPanel::AddMaterial(systems::FolderNode* node) {

		std::string matName = "New Material";
		m_materialRegistry.AddNewMaterial(node, matName);

		m_materialRegistry.OpenFolderNode(node);
		m_setFolderOpenNextFrame = true;
		m_folderIdToOpenNextFrame = node->runtime_ID;

	}
	void MaterialLibraryGuiPanel::RenameMaterial(systems::FolderNode* node, systems::MaterialInfo& materialInfo, std::string& newName, int positionInVector) {

		if (newName != materialInfo.name) {
			m_materialRegistry.RenameMaterial(node, materialInfo, newName,positionInVector);
		}
	}

	void MaterialLibraryGuiPanel::DeleteMaterial(systems::FolderNode* node, systems::MaterialInfo& materialInfo, int positionInVector) {
		m_materialRegistry.DeleteMaterial(node, materialInfo, positionInVector);
	}
} // !mnemosy::gui