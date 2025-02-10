#include "Include/GuiPanels/MaterialLibraryGuiPanel.h"

#include "Include/ApplicationConfig.h"
#include "Include/GuiPanels/GuiPanelsCommon.h"

#include "Include/MnemosyEngine.h"

#include "Include/Core/Log.h"
#include "Include/Core/FileDirectories.h"

#include "Include/Systems/MaterialLibraryRegistry.h"
#include "Include/Systems/FolderTreeNode.h"
#include "Include/Systems/ThumbnailManager.h"

#include "Include/Graphics/Renderer.h"

#ifdef MNEMOSY_PLATFORM_WINDOWS
#include "Include/Core/Utils/PlatfromUtils_Windows.h"
#endif // MNEMOSY_PLATFORM_WINDOWS


#include <string>
#include <filesystem>
#include <iostream>


namespace mnemosy::gui
{
	MaterialLibraryGuiPanel::MaterialLibraryGuiPanel()
		: m_materialRegistry { MnemosyEngine::GetInstance().GetMaterialLibraryRegistry()}
	{
		panelName = "Library Hierarchy";
		panelType = MNSY_GUI_PANEL_MATERIAL_LIBRARY;

		//rootNode = m_materialRegistry.GetRootFolder();
		//m_materialRegistry.OpenFolderNode(rootNode);


		m_directoryTreeFlags = ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_SpanAllColumns | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_OpenOnArrow;
		m_materialTreeFlags = ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_SpanAllColumns | ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_Bullet;

	}


	void MaterialLibraryGuiPanel::Draw() {
		
		if (!showPanel)
			return;

		ImGui::Begin(panelName, &showPanel);
		{
			
			
			ImGui::Spacing();
			ImGui::Spacing();

			DrawLibrarySelection();


			ImGui::Spacing();
			ImGui::Spacing();

			ImGui::Separator();


			// return early if no library is selected

			if (!m_materialRegistry.LibCollections_IsAnyActive()) {

				ImGui::Text("No Library is currently loaded.");


				ImGui::End();
				return;
			}

			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::Spacing();

			ImGui::Text("Search: ");
			ImGui::SameLine();

			// === Search Bar
			bool searchEnter = ImGui::InputText("##SearchInputField", &m_searchInput, m_textInputFlags);
			if (searchEnter) {

				if (m_searchInput == "") {

					m_materialRegistry.inSearchMode = false;
					m_materialRegistry.OpenFolderNode(m_materialRegistry.GetRootFolder());
				}
				else {
					

					bool searchResultsFound = m_materialRegistry.SearchLibEntriesForKeyword(m_searchInput);
					m_materialRegistry.inSearchMode = true;


					systems::ThumbnailManager& thumbManager = MnemosyEngine::GetInstance().GetThumbnailManager();

					thumbManager.UnloadAllThumbnails();

					if (searchResultsFound) {
						
						std::vector<systems::LibEntry*>& searchResultsList = m_materialRegistry.GetSearchResultsList();


						for (unsigned int i = 0; i < searchResultsList.size(); i++) {
							
							thumbManager.AddLibEntryToActiveThumbnails(searchResultsList[i]);							
						}						
					}

				}

			}

			ImGui::Spacing();
			ImGui::Spacing();





			if (!m_materialRegistry.inSearchMode) {

				if (m_materialRegistry.LibCollections_IsAnyActive()) {

					RecursivDrawSubfolders(m_materialRegistry.GetRootFolder());
					PopupModal_Folder_DeleteHierarchy();
				}
			}
			else {

				// draw search results as bullet points
				std::vector<systems::LibEntry*>& searchResultsList = m_materialRegistry.GetSearchResultsList();

				if (!searchResultsList.empty()) {

					for (unsigned int i = 0; i < searchResultsList.size(); i++) {
						
						if (ImGui::TreeNodeEx(searchResultsList[i]->name.c_str(), m_materialTreeFlags)) {
							
							ImGui::TreePop();
						}
					}
				}
				else {
						
					ImGui::Text("Could not find any materials..");
					
				}
			}
			

		}	
		ImGui::End();

	}

	void MaterialLibraryGuiPanel::RecursivDrawSubfolders(systems::FolderNode* node) {
		
		if (node == nullptr)
			return;
		
		// prepare node flags
		ImGuiTreeNodeFlags node_flags = m_directoryTreeFlags;


		std::string displayName = node->name;

		if (node->IsRoot()) {
			node_flags |= ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed;// ImGuiTreeNodeFlags_CollapsingHeader;
			//node_flags = ImGuiTreeNodeFlags_CollapsingHeader;
			displayName = m_materialRegistry.ActiveLibCollection_GetName();
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

		bool nodeOpen = ImGui::TreeNodeEx(displayName.c_str(), node_flags);
			
		
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

					gui::GuiProcedures::folder_rename(node,m_renameFolderText);

				}
			}



			for (int i = 0; i < IM_ARRAYSIZE(m_rightClickFolderOptions); i++) {


				// make entry types use different text colors
				ImVec4 txtColor = TextColor_default; // Default
				if (i == 1) { // pbr
					txtColor = TextColor_pbr; // PBR

				}
				else if (i == 2) {

					txtColor = TextColor_unlit; // Texture
				}
				else if (i == 3) {

					txtColor = TextColor_skybox; // Skybox
				}

				ImGui::PushStyleColor(ImGuiCol_Text, txtColor);


				if (ImGui::Selectable(m_rightClickFolderOptions[i])) {

					
					if (i == 0) { // Add Subfolder
						
						gui::GuiProcedures::folder_create_new(node,true);

						m_setFolderOpenNextFrame = true;
						m_folderIdToOpenNextFrame = node->runtime_ID;

					}
					else if (i == 1) { // Add PBR material 

						gui::GuiProcedures::libEntry_create_new(node,"New Material",systems::LibEntryType::MNSY_ENTRY_TYPE_PBRMAT,true);

						m_setFolderOpenNextFrame = true;
						m_folderIdToOpenNextFrame = node->runtime_ID;

						//AddMaterialEntry(node, "New Material", systems::LibEntryType::MNSY_ENTRY_TYPE_PBRMAT);
					}
					else if (i == 2) { // Add Unlit Texture
					
						gui::GuiProcedures::libEntry_create_new(node,"New Texture",systems::LibEntryType::MNSY_ENTRY_TYPE_UNLITMAT,true);
						//AddMaterialEntry(node, "New Texture", systems::LibEntryType::MNSY_ENTRY_TYPE_UNLITMAT);
						m_setFolderOpenNextFrame = true;
						m_folderIdToOpenNextFrame = node->runtime_ID;
					}
					else if (i == 3) { // Add skybox
						gui::GuiProcedures::libEntry_create_new(node,"New Skybox",systems::LibEntryType::MNSY_ENTRY_TYPE_SKYBOX,true);
						
						m_setFolderOpenNextFrame = true;
						m_folderIdToOpenNextFrame = node->runtime_ID;
					
					}
					else if (i == 4) { // delete but keep children

						gui::GuiProcedures::folder_delete_move_children_to_parent(node);
					}
					else if (i == 5) { // delete hierarchy
						
						PopupModal_Folder_DeleteHierarchy_Open(node);
					}
					else if (i == 6) { // open in explorer
						
						//fs::path pathToFolder = m_materialRegistry.Folder_GetFullPath(node);
						mnemosy::core::FileDialogs::OpenFolderAt(m_materialRegistry.Folder_GetFullPath(node));
					}

				}

				ImGui::PopStyleColor();


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
						systems::FolderNode* sourceNode = materialPayload.sourceNode;

						MNEMOSY_ASSERT(sourceNode != nullptr, "A nullpointer folder node should never be passed into a drag and drop payload");

						// need to make a copy of node.submaterials because it changed during the loop
						std::vector<systems::LibEntry*> subMatsCopy = sourceNode->subEntries;

						for (unsigned int b = 0; b < materialPayload.matList->materialListIndexes.size(); b++) {

							unsigned int index = materialPayload.matList->materialListIndexes[b];
							//MNEMOSY_TRACE("Material index: {}", index);
							systems::LibEntry* matInfoTemp = subMatsCopy[index];
							m_materialRegistry.LibEntry_Move(sourceNode, node, matInfoTemp);
						}
						
						subMatsCopy.clear();
					}


					ImGui::EndDragDropTarget();
				}
		
		} // Drag and drop

		if (nodeOpen) {
			

			// === Recusivly draw Sub directories

			if (!node->IsLeafNode()) {
				for (unsigned int i = 0; i < node->subNodes.size(); i++) {
					RecursivDrawSubfolders(node->subNodes[i]);
				}
			}
			
			// === Draw Material Entries
			Draw_LibEntries(node);

		
			ImGui::TreePop();
		}
		
	}

	void MaterialLibraryGuiPanel::Draw_LibEntries(systems::FolderNode* node) {

		if (node->subEntries.empty())
			return;

		for (unsigned int i = 0; i < node->subEntries.size(); i++) {
				
			std::string materialText = node->subEntries[i]->name;



			// make entry types use different text colors
			ImVec4 txtColor = TextColor_pbr; // PBR
			if (node->subEntries[i]->type == systems::LibEntryType::MNSY_ENTRY_TYPE_UNLITMAT) {
				txtColor = TextColor_unlit;
			}
			else if (node->subEntries[i]->type == systems::LibEntryType::MNSY_ENTRY_TYPE_SKYBOX) {

				txtColor = TextColor_skybox;
			}

			ImGui::PushStyleColor(ImGuiCol_Text, txtColor);


			bool selected = ImGui::Selectable(materialText.c_str(), node->subEntries[i]->selected);


			ImGui::PopStyleColor();

			if (ImGui::IsItemClicked()) {
			
				// we need this here to reset it if user dragged but it didnt arrive
				m_matDragDropBegin = false;
			}

			if (selected) {

				if (!ImGui::GetIO().KeyCtrl) {
					// clear selection 
					for (unsigned int a = 0; a < node->subEntries.size(); a++) {

						node->subEntries[a]->selected = false;
					}
				}

				node->subEntries[i]->selected = true;	

			}


			if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) {

				if(!m_matDragDropBegin){
					
					m_matDragDropBegin = true;

					// current mat will be selected
					node->subEntries[i]->selected = true;

					MaterialDragDropPayload matPayload = MaterialDragDropPayload();
					matPayload.sourceNode = node;
					matPayload.matList = std::make_shared<PointerList>();


					for (unsigned int a = 0; a < node->subEntries.size(); a++) {

						if (node->subEntries[a]->selected) {

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



			// === Right Click Options
			// right click on open folder to open options
			{





				if (ImGui::BeginPopupContextItem()) {

					m_renameMaterialText = node->subEntries[i]->name;

					ImGui::Text("Rename: ");
					ImGui::SameLine();

					bool renamed = ImGui::InputText("##RenameMaterialInputField", &m_renameMaterialText,m_textInputFlags);
					if (renamed) {

						gui::GuiProcedures::libEntry_rename(node->subEntries[i],m_renameMaterialText);
					}

					for (unsigned int option = 0; option < IM_ARRAYSIZE(m_rightClickMaterialOptions); option++) {

						if (ImGui::Selectable(m_rightClickMaterialOptions[option])) {
							
							if (option == 0) { // load
							

								m_materialRegistry.LibEntry_Load(node->subEntries[i]);
							
							}
							else if (option == 1) { // delete
								
								// dude what the hell is going on here.. maybe i should just delay this to next frame..
								std::vector<systems::LibEntry*> subMatsCopy = node->subEntries;

								for (unsigned int a = 0; a < subMatsCopy.size(); a++) {


									if (subMatsCopy[a]->selected) {

										int posInList = -1;
										// find posiiton in the original list which is changing as we delete materials
										int runtimeID = subMatsCopy[a]->runtime_ID;
										for (unsigned int b = 0; b < node->subEntries.size(); b++) {

											if (runtimeID == node->subEntries[b]->runtime_ID) {

												posInList = b;
											}

										}

										MNEMOSY_ASSERT(posInList > -1, "Should always find the correct list entry");

										gui::GuiProcedures::libEntry_delete(subMatsCopy[a],posInList);
									}

								}

								subMatsCopy.clear();
							} 
							
							else if (option == 2) { // open in FileExplorer

								// opens folder of the material in system explorer
								mnemosy::core::FileDialogs::OpenFolderAt(m_materialRegistry.LibEntry_GetFolderPath(node->subEntries[i]));
							}
						}



					} // end options loop
					ImGui::EndPopup();
				} // End Popup Context Item


		


				//ImGui::TreePop();
			}




		} // end loop through sub materials
	}



	void MaterialLibraryGuiPanel::PopupModal_Folder_DeleteHierarchy_Open(systems::FolderNode* folder) {

		m_popupModal_folder_deleteHierarchy_triggered = true;
		m_popupModal_folder_deleteHierarchy_ptr = folder;
	}

	void MaterialLibraryGuiPanel::PopupModal_Folder_DeleteHierarchy() {

		static bool popModal_deleteHierarchy_show = false;
		if (m_popupModal_folder_deleteHierarchy_triggered) {

			popModal_deleteHierarchy_show = true;
			m_popupModal_folder_deleteHierarchy_triggered = false; // to make sure its only called once
			ImGui::OpenPopup("Delete Folder Hierarchy");
		}

		if (ImGui::BeginPopupModal("Delete Folder Hierarchy", &popModal_deleteHierarchy_show, ImGuiWindowFlags_AlwaysAutoResize)) {

			ImGui::Text("Are you sure? \nAll files in this folder will be Deleted permanently!");

			if (ImGui::Button("Cancel", ImVec2(120, 0))) {
				m_popupModal_folder_deleteHierarchy_ptr = nullptr;
				popModal_deleteHierarchy_show = false;
				ImGui::CloseCurrentPopup();
			}

			ImGui::SameLine();

			if (ImGui::Button("Delete Permanently")) {

				if (m_popupModal_folder_deleteHierarchy_ptr) {

					gui::GuiProcedures::folder_delete_hierarchy(m_popupModal_folder_deleteHierarchy_ptr);
				}
				
				
				popModal_deleteHierarchy_show = false;
				m_popupModal_folder_deleteHierarchy_ptr = nullptr;
				ImGui::CloseCurrentPopup();
			}

			ImGui::EndPopup();
		}


	}

	void MaterialLibraryGuiPanel::DrawLibrarySelection() {



		//if (ImGui::Button("Add Library##Btn")) {

		//	m_openAddLibrary_popup = true;
		//}


		ImGui::SameLine();


		// ==== Combo selection
		const std::vector<systems::LibCollection>& list = m_materialRegistry.LibCollections_GetListVector();

		unsigned int current_selection_id = m_materialRegistry.LibCollections_GetCurrentSelectedID();

		if (!list.empty() && current_selection_id != -1) {


			
			const char* combo_preview_value = list[current_selection_id].name.c_str();



			unsigned int popupOption = 0;
			unsigned int popupEntryIndex = 0;

			ImGui::SetNextItemWidth(150);

			if (ImGui::BeginCombo(" ##ViewportSkybox", combo_preview_value, 0))
			{
				for (uint16_t n = 0; n < list.size(); n++)
				{
					const bool is_selected = (current_selection_id == n);
					if (ImGui::Selectable(list[n].name.c_str(), is_selected)) {
						current_selection_id = n;
					}


					// popup selection
					if (ImGui::BeginPopupContextItem()) {


						// TODO: rename option

						m_renameLibCollectionText = list[n].name;

						ImGui::Text("Rename: ");
						ImGui::SameLine();

						bool renamed = ImGui::InputText("##RenameCollectionInputField", &m_renameLibCollectionText, m_textInputFlags);
						if (renamed) {

							//popupOptionSelected = true;
							popupEntryIndex = n;
							popupOption = 1;

						}



						if (ImGui::Selectable("Remove from List")) {

							popupOption = 2;
							//popupOptionSelected = true;
							popupEntryIndex = n;
						}


						ImGui::EndPopup();
					}

					// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
					if (is_selected)
						ImGui::SetItemDefaultFocus();
				}

				ImGui::EndCombo();
			}


			if (popupOption != 0) {


				if (popupOption == 1) { // rename seleciton


					m_materialRegistry.LibCollections_RenameEntry(popupEntryIndex, m_renameLibCollectionText);

				}
				else if (popupOption == 2) { // remove entry from list.

					m_materialRegistry.LibCollections_RemoveEntryFromList(popupEntryIndex);
				}
			}
			else if (current_selection_id != m_materialRegistry.LibCollections_GetCurrentSelectedID()) {

				m_materialRegistry.LibCollections_SwitchActiveCollection(current_selection_id);
			}

		}


		ImGui::SameLine();
		
		
		static bool m_openAddLibrary_popup = false;
		static bool m_addLib_popup_isActive = false;

		if (ImGui::Button("Add Library##Btn")) {

			m_openAddLibrary_popup = true;
		}



		// ===  Add library Popup modal.

		if (m_openAddLibrary_popup) {
			m_openAddLibrary_popup = false;
			m_addLib_popup_isActive = true;
			ImGui::OpenPopup("Add Library");
		}

		if (ImGui::BeginPopupModal("Add Library", &m_addLib_popup_isActive, ImGuiWindowFlags_AlwaysAutoResize))
		{
			ImGui::Spacing();


			static bool addExisting = false;
			ImGui::Text("Add Existing Mnemosy Library: ");
			ImGui::SameLine();
			ImGui::Checkbox("##AddExisting", &addExisting);
			ImGui::SetItemTooltip("Wheather to add the new library from an existing mnemosy library directory or create a new empty one.");

			ImGui::Spacing();

			// Lib Name

			static std::string addLib_newNameInput = "New Library";

			ImGui::Text("Library Name: ");
			ImGui::SameLine();

			bool renamed = ImGui::InputText("##LibName", &addLib_newNameInput, m_textInputFlags);
			if (renamed) {
				addLib_newNameInput = m_materialRegistry.LibCollections_MakeNameUnique(addLib_newNameInput);
			}

			ImGui::Spacing();


			// filepath 
			static std::filesystem::path filepath = std::filesystem::path("C:/");


			if (addExisting) {
				ImGui::Text("Select the 'MnemosyMaterialLibraryData.mnsydata' file of an existing Mnemosy Libray");
				
				ImGui::Text("Path: %s", filepath.generic_string().c_str());

				if (ImGui::Button("Select .mnsydata File...##AddLib")) {

					std::filesystem::path p = mnemosy::core::FileDialogs::OpenFile("mnsydata (*.mnsydata)\0*.mnsydata\0");

					if (!p.empty()) {
						filepath = p;
					}
				}
			
			}
			else {
				ImGui::Text("Select an empty folder for the new Mnemosy Libray");

				ImGui::Text("Path: %s", filepath.generic_string().c_str());

				if (ImGui::Button("Select Folder...##AddLib")) {

					std::filesystem::path p = mnemosy::core::FileDialogs::SelectFolder("");
				
					if (!p.empty()) {
						filepath = p;
					}
				}
			}

			ImGui::Spacing();
			ImGui::Spacing();



			if (ImGui::Button("Cancel", ImVec2(120, 0) )  ) {

				m_addLib_popup_isActive = false;
				ImGui::CloseCurrentPopup();
			}

			ImGui::SameLine();

			if (ImGui::Button("Add", ImVec2(120, 0) )) {

				if (addExisting) {

					m_materialRegistry.LibCollections_CreateNewEntryFromExisting(addLib_newNameInput,filepath);
				}
				else {
					m_materialRegistry.LibCollections_CreateNewEntry(addLib_newNameInput,filepath);
				}





				m_addLib_popup_isActive = false;
				ImGui::CloseCurrentPopup();
			}

			

			ImGui::EndPopup();
		}


		




	}

} // !mnemosy::gui