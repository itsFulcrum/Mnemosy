#include "Include/GuiPanels/ContentsGuiPanel.h"

#include "Include/MnemosyEngine.h"
#include "Include/ApplicationConfig.h"
#include "Include/MnemosyConfig.h"
#include "Include/GuiPanels/GuiPanelsCommon.h"

#include "Include/Gui/UserInterface.h"

#include "Include/Core/Log.h"
#include "Include/Core/FileDirectories.h"
#include "Include/Systems/MaterialLibraryRegistry.h"
#include "Include/Systems/FolderTreeNode.h"
#include "Include/Core/Utils/StringUtils.h"
#include "Include/Graphics/Texture.h"
#include "Include/Graphics/Utils/Picture.h"


#ifdef MNEMOSY_PLATFORM_WINDOWS
#include "Include/Core/Utils/PlatfromUtils_Windows.h"
#endif // MNEMOSY_PLATFORM_WINDOWS

#include <string>
#include <vector>
#include <filesystem>

namespace mnemosy::gui {


	ContentsGuiPanel::ContentsGuiPanel()
		: m_materialRegistry{ MnemosyEngine::GetInstance().GetMaterialLibraryRegistry() }
	{
		panelName = "Contents";
		panelType = MNSY_GUI_PANEL_CONTENTS;

		m_payload_libEntry_ptr = new gui::LibEntry_Payload();
		m_payload_folder_ptr = new gui::Folder_Payload();


		// load folder icon texture
		{

			namespace fs = std::filesystem;

			fs::path texPath = MnemosyEngine::GetInstance().GetFileDirectories().GetTexturesPath() / fs::path("Icons/Material_Symbols_Folder_Open.png");

			graphics::PictureError err;

			graphics::PictureInfo picInfo = graphics::Picture::ReadPicture(err,texPath.generic_string().c_str(),true,false,false);

			if (err.wasSuccessfull) {


				// could store this with arena allocator
				m_folder_icon_tex = new graphics::Texture();
				m_folder_icon_tex->GenerateOpenGlTexture(picInfo,true);

				if (picInfo.pixels) {
					free(picInfo.pixels);
				}
			}
			else {
				MNEMOSY_ERROR("Faild to load Icons texture: Message: {}", err.what);
				return;
			}
			
		}
	}

	ContentsGuiPanel::~ContentsGuiPanel()
	{

		if (m_folder_icon_tex) {
			delete m_folder_icon_tex;
		}

		if (m_payload_folder_ptr) {
			delete m_payload_folder_ptr;
		}

		if (m_payload_libEntry_ptr) {
			delete m_payload_libEntry_ptr;
		}

	}

	void ContentsGuiPanel::Draw()
	{
		if (!showPanel)
			return;

		// updates
		{
			PopupModal_LibEntry_Delete();

			PopupModal_Folder_DeleteHierachy();

		
			// we have to delay deltions to next frame so we check here if 'm_delete_folder_next_frame != nullptr' then we know there is something qued for delete
			Folder_DeleteButKeepChildren_Next_Frame(nullptr, true);

		}

		ImGui::Begin(panelName, &showPanel);
		{
			if (m_materialRegistry.LibCollections_IsAnyActive()) {


				if (!m_materialRegistry.inSearchMode) {

					DrawMaterialButtons();
				}
				else {

					DrawMaterialButtonsOfSearch();
				}
			}


		} ImGui::End();
	}


	void ContentsGuiPanel::DrawMaterialButtons() {

		namespace fs = std::filesystem;

		if (m_materialRegistry.GetSelectedNode() == nullptr) {
			MNEMOSY_TRACE("Selected node is nullptr");
		}

		systems::FolderNode* selectedNode = m_materialRegistry.GetSelectedNode();
		// we dont need to check for selectedNode nullptr because we ensure that there always is one selected

		if (selectedNode->runtime_ID != m_currentFolder_id) {
			m_currentFolder_id = selectedNode->runtime_ID;
		}

		// ===========================================================================================
		// =========================== HEADER ========================================================
		// ===========================================================================================
		
		if (!selectedNode->IsRoot()) {			

			const static std::string btn_arrow_up = core::StringUtils::wChar_to_utf8String(0xE5F2);

			ImGui::PushFont(MnemosyEngine::GetInstance().GetUserInterface().Font_Icon_Get());

			if (ImGui::Button(btn_arrow_up.c_str())) {

				m_materialRegistry.OpenFolderNode(selectedNode->parent);
			}
			ImGui::PopFont();
		}


		ImGui::SameLine();

		ImGui::Text("Directory: %s", m_materialRegistry.GetSelectedNode()->name.c_str());
		ImGui::SameLine();

		if (ImGui::SliderFloat("Icon Size", &m_imgButtonSize, 32.0f, 350.0f, "%.0f")) {

			m_image_button_size.x = m_imgButtonSize;
			m_image_button_size.y = m_imgButtonSize;
		}
		
		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();


		ImGuiStyle& style = ImGui::GetStyle();
		float window_visible_x2 = ImGui::GetWindowPos().x + ImGui::GetWindowContentRegionMax().x;


		// ===========================================================================================
		// =========================== GLOBAL RIGHT CLICK MENU =======================================
		// ===========================================================================================

		static bool inContextMenu = false;
		bool isAnyItemOvered = ImGui::IsAnyItemHovered();

		if (!isAnyItemOvered || inContextMenu) {

			if (ImGui::BeginPopupContextWindow("FolderPopup") ) {

				inContextMenu = true;	
				

				if (ImGui::Selectable("Add Subfolder")) {

					std::string newName = "New Folder";
					systems::FolderNode* newFolder = m_materialRegistry.AddNewFolder(selectedNode, newName);
				}

				ImGui::PushStyleColor(ImGuiCol_Text, TextColor_pbr);
				if (ImGui::Selectable("Add PBR Material")) {
					gui::GuiProcedures::libEntry_create_new(selectedNode, "New Material", systems::LibEntryType::MNSY_ENTRY_TYPE_PBRMAT, false);
				}
				ImGui::PopStyleColor();

				
				ImGui::PushStyleColor(ImGuiCol_Text, TextColor_unlit);
				if (ImGui::Selectable("Add Unlit Texture")) {
					gui::GuiProcedures::libEntry_create_new(selectedNode, "New Texture", systems::LibEntryType::MNSY_ENTRY_TYPE_UNLITMAT,false);
				}
				ImGui::PopStyleColor();

				
				ImGui::PushStyleColor(ImGuiCol_Text, TextColor_skybox);
				if (ImGui::Selectable("Add Skybox")) {
					gui::GuiProcedures::libEntry_create_new(selectedNode, "New Skybox", systems::LibEntryType::MNSY_ENTRY_TYPE_SKYBOX, false);
				}
				ImGui::PopStyleColor();


				if (ImGui::Selectable("Open System Folder")) {
					mnemosy::core::FileDialogs::OpenFolderAt(m_materialRegistry.Folder_GetFullPath(selectedNode));
				}

				ImGui::EndPopup();
		
			}	
		}

		inContextMenu = ImGui::IsPopupOpen("FolderPopup");


		// ===========================================================================================
		// =========================== DRAW FOLDERS ==================================================
		// ===========================================================================================
		
		unsigned int folderCount = selectedNode->subNodes.size();
		unsigned int matEntriesCount = selectedNode->subEntries.size();
		unsigned int buttonsCount = folderCount + matEntriesCount;

		if (folderCount > 0) {

			for (unsigned int i = 0; i < selectedNode->subNodes.size(); i++) {
			
				systems::FolderNode* curr_folder = selectedNode->subNodes[i];

				ImGui::BeginGroup();
				{

					std::string& folderName = curr_folder->name;

					uint64_t textureId = 0;
					if (m_folder_icon_tex) {
						textureId = (uint64_t)m_folder_icon_tex->GetID();
					}
					
					ImGui::PushID(folderName.c_str()); // we need this bc imageButton uses texture id as the id of the widgit which for folders will be the same..
					
					bool pressed = ImGui::ImageButton((void*)textureId, m_image_button_size, ImVec2(0, 1), ImVec2(1, 0),-1,ImVec4(0.1f, 0.1f, 0.1f, 1.0f), ImVec4(0.8f, 0.8f, 0.8f, 1.0f));


					if (pressed) {					
						m_materialRegistry.OpenFolderNode(curr_folder);
					}


					// ==================================================
					// ========== FOLDER DRAG DROP TARGET ===============
					// ==================================================

					if (ImGui::BeginDragDropTarget()) {


						// == lib entry target
						if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Payload_SingleLibEntry")) {

							LibEntry_Payload* entry_Payload = static_cast<LibEntry_Payload*>(payload->Data);

							systems::LibEntry* libEntry_payload = entry_Payload->libEntry;


							if (libEntry_payload) {

								m_materialRegistry.LibEntry_Move(libEntry_payload->parent,curr_folder ,libEntry_payload);
							}
						}
						
						// == Folder target

						if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Payload_SingleFolder")) {

							gui::Folder_Payload* folder_payload = static_cast<gui::Folder_Payload*>(payload->Data);

							systems::FolderNode* folderNode_payload = folder_payload->folderNode;

							if (folderNode_payload) {


								m_materialRegistry.MoveFolder(folderNode_payload,curr_folder);
							}
						}

						ImGui::EndDragDropTarget();
					}

					// ==================================================
					// ========== FOLDER DRAG DROP SOURCE ===============
					// ==================================================

					if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) {

						m_payload_folder_ptr->folderNode = curr_folder;

						ImGui::SetDragDropPayload("Payload_SingleFolder", static_cast<void*>(m_payload_folder_ptr), sizeof(gui::Folder_Payload));

						ImGui::EndDragDropSource();
					}

					// ==================================================
					// ========= RIGHT CLICK OPTIONS FODLER =============
					// ==================================================

					if (ImGui::BeginPopupContextItem("FolderPopContext")) {

						m_rename_input_text = folderName;

						ImGui::Text("Rename: ");
						ImGui::SameLine();

						bool renamed = ImGui::InputText("##RenameFolderInput", &m_rename_input_text, m_textInputFlags);
						if (renamed) {							
							gui::GuiProcedures::folder_rename(curr_folder,m_rename_input_text);
						}
						
						if (ImGui::Selectable("Delete##F")) {

							Folder_DeleteButKeepChildren_Next_Frame(curr_folder, false);
						}

						if (ImGui::Selectable("Delete Hierachy##F")) {

							PopupModal_Folder_DeleteHierachy_Open(curr_folder);				
						}

						if (ImGui::Selectable("Open System Folder##F")) {

							mnemosy::core::FileDialogs::OpenFolderAt(m_materialRegistry.Folder_GetFullPath(curr_folder));
						}

						ImGui::EndPopup();
					}

					ImGui::PopID(); // Pop Img button id


					std::string folderNameString = folderName;
					ShortenNameStringToFitButtonSize(folderNameString, m_imgButtonSize);

					ImGui::Text(folderNameString.c_str());

				}
				ImGui::EndGroup();


				float last_button_x2 = ImGui::GetItemRectMax().x;
				float next_button_x2 = last_button_x2 + style.ItemSpacing.x + m_image_button_size.x;

				if (i + 1 < buttonsCount && next_button_x2 < window_visible_x2) {
					ImGui::SameLine();
				}

			}			
		}


		// ===========================================================================================
		// =========================== DRAW MATERIAL ENTRIES =========================================
		// ===========================================================================================
		
		if (matEntriesCount > 0) {			
			for (unsigned int i = 0; i < selectedNode->subEntries.size(); i++) {

				systems::LibEntry* curr_libEntry = selectedNode->subEntries[i];

				ImGui::BeginGroup();
				{
					std::string& entryName = curr_libEntry->name;


					// first casting to a uint64 is neccesary to get rid of waring for casting to void* in next line
				
					uint64_t thumb_tex_id = (uint64_t)curr_libEntry->thumbnailTexure_ID;
					if (!curr_libEntry->thumbnailLoaded) {
						thumb_tex_id = 0;
					}

					ImGui::PushID(entryName.c_str());

					bool pressed = ImGui::ImageButton((void*)thumb_tex_id, m_image_button_size, ImVec2(0, 1), ImVec2(1, 0));
					
					if (pressed) {

						// check if its already the active material
						if (!m_materialRegistry.IsActiveEntry(curr_libEntry->runtime_ID)) {
							m_materialRegistry.LibEntry_Load(curr_libEntry);
						}
					}

					// ==================================================
					// ========== DRAG DROP SOURCE ======================
					// ==================================================

					if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) {

						m_payload_libEntry_ptr->libEntry = curr_libEntry;

						ImGui::SetDragDropPayload("Payload_SingleLibEntry", static_cast<void*>(m_payload_libEntry_ptr), sizeof(LibEntry_Payload));

						ImGui::EndDragDropSource();
					}


					// ==================================================
					// ========= RIGHT CLICK OPTIONS ====================
					// ==================================================

					if (ImGui::BeginPopupContextItem("EntryPopContext")) {

						m_rename_input_text = entryName;

						ImGui::Text("Rename: ");
						ImGui::SameLine();

						bool renamed = ImGui::InputText("##RenameEntryInput", &m_rename_input_text, m_textInputFlags);
						if (renamed) {

							gui::GuiProcedures::libEntry_rename(curr_libEntry,m_rename_input_text);
						}

						if (ImGui::Selectable("Delete Permanently##E")) {

							PopupModel_LibEntry_Delete_Open(curr_libEntry);
						}


						if (ImGui::Selectable("Open System Folder##E")) {

							core::FileDialogs::OpenFolderAt(m_materialRegistry.LibEntry_GetFolderPath(curr_libEntry));
						}


						ImGui::EndPopup();
					}



					ImGui::PopID(); // Pop Image button ID


					// == Name undeneith button
					{
						// Calculate size of the name and shorten it if its longer then the button size
						std::string matNameString = entryName;
						ShortenNameStringToFitButtonSize(matNameString,m_imgButtonSize);

						ImGui::Text(matNameString.c_str());
					}

				}
				ImGui::EndGroup();

				float last_button_x2 = ImGui::GetItemRectMax().x;
				float next_button_x2 = last_button_x2 + style.ItemSpacing.x + m_image_button_size.x;
				
				if (i + 1 < buttonsCount && next_button_x2 < window_visible_x2) {
					ImGui::SameLine();
				}
			}
		}
		

		// ===========================================================================================
		// =========================== DRAG SELCTION (feature for the future) ========================
		// ===========================================================================================


		//ImDrawList* draw_list =  ImGui::GetWindowDrawList();
		//ImU32 col32 = ImColor(ImVec4(1.0f, 1.0f, 1.0f, 1.0f));

		//draw_list->AddRectFilled(ImVec2(50,50), ImVec2(100, 100),col32,0.0f );

		//ImVec2 panelPos = ImGui::GetCursorScreenPos();

		//ImVec2 mouse = ImGui::GetMousePos();

		//ImGui::SetNextItemAllowOverlap();
		//ImGui::GetWindowDrawList()->AddRectFilled(mouse, ImVec2(mouse.x + 50, mouse.y + 50), IM_COL32_WHITE);


	}

	void ContentsGuiPanel::DrawMaterialButtonsOfSearch() {

		namespace fs = std::filesystem;

		ImGui::Text("Search Results:");
		ImGui::SameLine();
		if(ImGui::SliderFloat("Icon Size", &m_imgButtonSize, 32.0f, 350.0f, "%.0f"))
		{
			m_image_button_size.x = m_imgButtonSize;
			m_image_button_size.y = m_imgButtonSize;
		}

		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();


		std::vector<systems::LibEntry*>& searchResultsList = m_materialRegistry.GetSearchResultsList();

		if (!searchResultsList.empty()) {


			ImGuiStyle& style = ImGui::GetStyle();
			float window_visible_x2 = ImGui::GetWindowPos().x + ImGui::GetWindowContentRegionMax().x;

			unsigned int entryCount = searchResultsList.size();

			for (unsigned int i = 0; i < entryCount; i++) {



				ImGui::BeginGroup();
				{

					systems::LibEntry* curr_libEntry = searchResultsList[i];

					std::string& curr_entryName = curr_libEntry->name;


					uint64_t thumb_tex_id = (uint64_t)curr_libEntry->thumbnailTexure_ID;
					if (!curr_libEntry->thumbnailLoaded) {
						thumb_tex_id = 0;
					}


					ImGui::PushID(curr_entryName.c_str());

					bool pressed = ImGui::ImageButton((void*)thumb_tex_id, m_image_button_size, ImVec2(0, 1), ImVec2(1, 0));

					if (pressed) {

						// check if its already the active material
						if (!m_materialRegistry.IsActiveEntry(curr_libEntry->runtime_ID)) {

							m_materialRegistry.LibEntry_Load(curr_libEntry);
						}
					}

					if (ImGui::BeginPopupContextItem("SearchContextMenu")) {



						m_rename_input_text = curr_entryName;

						ImGui::Text("Rename: ");
						ImGui::SameLine();

						bool renamed = ImGui::InputText("##RenameEntryInputSearch", &m_rename_input_text, m_textInputFlags);
						if (renamed) {

							gui::GuiProcedures::libEntry_rename(curr_libEntry, m_rename_input_text);
						}

						// requires to update the search results list AND probably should wait until next frame as the current list is a refrence not a copy
						//if (ImGui::Selectable("Delete Permanently##Search")) {

						//	//PopupModel_LibEntry_Delete_Open(curr_libEntry);
						//}

						if (ImGui::Selectable("Open System Folder##Search")) {

							core::FileDialogs::OpenFolderAt(m_materialRegistry.LibEntry_GetFolderPath(curr_libEntry));
						}

						ImGui::EndPopup();
					}





					ImGui::PopID(); // Pop Image Button ID


					// Calculate size of the name and shorten it if its longer then the button size
					std::string curr_entryNameCopy = curr_entryName;
					ShortenNameStringToFitButtonSize(curr_entryNameCopy,m_imgButtonSize);


					ImGui::Text(curr_entryNameCopy.c_str());

				}
				ImGui::EndGroup();

				float last_button_x2 = ImGui::GetItemRectMax().x;
				float next_button_x2 = last_button_x2 + style.ItemSpacing.x + m_image_button_size.x;

				if (i + 1 < entryCount && next_button_x2 < window_visible_x2) {
					ImGui::SameLine();
				}
			}
		}

	}




	// ===========================================================================================
	// =========================== HELPERS =======================================================
	// ===========================================================================================

	void ContentsGuiPanel::ShortenNameStringToFitButtonSize(std::string& str, float currentImageButtonSize) {

		if (ImGui::CalcTextSize(str.c_str()).x > (currentImageButtonSize - 5.0f)) {

			while (ImGui::CalcTextSize(str.c_str()).x > (currentImageButtonSize - 5.0f)) {
				str.pop_back();
			}

			str += "..";
		}
	}

	void ContentsGuiPanel::Folder_DeleteButKeepChildren_Next_Frame(systems::FolderNode* folder, bool isUpdateLoop) {

		if (isUpdateLoop) {
			
			if (m_folder_deleteNextFrame_ptr) {
				gui::GuiProcedures::folder_delete_move_children_to_parent(m_folder_deleteNextFrame_ptr);
				m_folder_deleteNextFrame_ptr = nullptr;
			}
		}
		else {

			m_folder_deleteNextFrame_ptr = folder;
		}
	}


	void ContentsGuiPanel::PopupModal_Folder_DeleteHierachy_Open(systems::FolderNode* folder) {

		m_popupModal_folder_deleteHierachy_triggered = true;
		m_popupModal_folder_deleteHierarchy_pbr = folder;
	}

	void ContentsGuiPanel::PopupModal_Folder_DeleteHierachy() {

		static bool popModal_DeleteFolderHierachy = false;
		if (m_popupModal_folder_deleteHierachy_triggered) {

			popModal_DeleteFolderHierachy = true;
			m_popupModal_folder_deleteHierachy_triggered = false; // to make sure its only called once
			ImGui::OpenPopup("Delete Hierarchy##FolderContents");
		}

		if (ImGui::BeginPopupModal("Delete Hierarchy##FolderContents", &popModal_DeleteFolderHierachy, ImGuiWindowFlags_AlwaysAutoResize)) {

			ImGui::Text("Are you sure? \nAll files in this folder will be deleted permanently!");

			if (ImGui::Button("Cancel", ImVec2(120, 0))) {
				m_popupModal_folder_deleteHierarchy_pbr = nullptr;
				popModal_DeleteFolderHierachy = false;
				ImGui::CloseCurrentPopup();
			}

			ImGui::SameLine();

			if (ImGui::Button("Delete Permanently")) {

				if (m_popupModal_folder_deleteHierarchy_pbr) {

					gui::GuiProcedures::folder_delete_hierarchy(m_popupModal_folder_deleteHierarchy_pbr);
				}

				popModal_DeleteFolderHierachy = false;
				m_popupModal_folder_deleteHierarchy_pbr = nullptr;
				ImGui::CloseCurrentPopup();
			}

			ImGui::EndPopup();
		}

	}


	void ContentsGuiPanel::PopupModel_LibEntry_Delete_Open(systems::LibEntry* entry) {

		m_popupModel_LibEntry_delete_triggered = true;
		m_popupModal_libEntry_delete_ptr = entry;
	}

	void ContentsGuiPanel::PopupModal_LibEntry_Delete() {

		static bool popModal_LibEntry_show = false;
		if (m_popupModel_LibEntry_delete_triggered) {

			m_popupModel_LibEntry_delete_triggered = false; // to make sure its only called once

			popModal_LibEntry_show = true;
			ImGui::OpenPopup("Delete Material?##Contents");
		}

		if (ImGui::BeginPopupModal("Delete Material?##Contents", &popModal_LibEntry_show, ImGuiWindowFlags_AlwaysAutoResize)) {

			ImGui::Text("Are you sure? \nMaterial will be deleted permanently!");

			if (ImGui::Button("Cancel", ImVec2(120, 0))) {

				m_popupModal_libEntry_delete_ptr = nullptr;
				popModal_LibEntry_show = false;
				ImGui::CloseCurrentPopup();
			}

			ImGui::SameLine();

			if (ImGui::Button("Delete Permanently")) {

				if (m_popupModal_libEntry_delete_ptr) {



					// find pos in parent vector;
					int indexPos = -1;
					for (int i = 0; i < m_popupModal_libEntry_delete_ptr->parent->subEntries.size(); i++) {

						if (m_popupModal_libEntry_delete_ptr->runtime_ID == m_popupModal_libEntry_delete_ptr->parent->subEntries[i]->runtime_ID) {

							indexPos = i;
							break;
						}
					}

					MNEMOSY_ASSERT(indexPos != -1, "It should always be found");

					gui::GuiProcedures::libEntry_delete(m_popupModal_libEntry_delete_ptr,static_cast<unsigned int>(indexPos));
				}

				

				popModal_LibEntry_show = false;
				m_popupModal_libEntry_delete_ptr = nullptr;
				ImGui::CloseCurrentPopup();
			}

			ImGui::EndPopup();
		}


	}




} // namespace end mnemosy::gui


