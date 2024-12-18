#include "Include/GuiPanels/ContentsGuiPanel.h"

#include "Include/MnemosyEngine.h"
#include "Include/ApplicationConfig.h"

#include "Include/Core/Log.h"
#include "Include/Core/FileDirectories.h"
#include "Include/Systems/MaterialLibraryRegistry.h"
#include "Include/Systems/FolderTreeNode.h"

#include "ImGui/imgui.h"
#include "ImGui/imgui_stdlib.h"

#include <string>
#include <vector>
#include <filesystem>

namespace mnemosy::gui {


	ContentsGuiPanel::ContentsGuiPanel()
		: m_materialRegistry{ MnemosyEngine::GetInstance().GetMaterialLibraryRegistry() }
	{
		panelName = "Contents";
		panelType = MNSY_GUI_PANEL_CONTENTS;
	}

	ContentsGuiPanel::~ContentsGuiPanel()
	{

	}

	void ContentsGuiPanel::Draw()
	{
		if (!showPanel)
			return;


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

		ImGui::Text("Directory: %s", m_materialRegistry.GetSelectedNode()->name.c_str());
		ImGui::SameLine();
		ImGui::SliderFloat("Icon Size", &m_imgButtonSize, 32.0f, 350.0f, "%.0f");

		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();



		if (m_materialRegistry.GetSelectedNode()->HasMaterials()) {
			
			fs::path lib = m_materialRegistry.ActiveLibCollection_GetFolderPath();
			fs::path activeFolderPath = lib / m_materialRegistry.GetSelectedNode()->GetPathFromRoot();



			systems::FolderNode* selectedNode = m_materialRegistry.GetSelectedNode();

			ImGuiStyle& style = ImGui::GetStyle();
			float window_visible_x2 = ImGui::GetWindowPos().x + ImGui::GetWindowContentRegionMax().x;

			ImVec2 button_size(m_imgButtonSize, m_imgButtonSize);
			int materialCount = selectedNode->subEntries.size();
			for (int i = 0; i < materialCount; i++) {

				ImGui::BeginGroup();
				std::string& matName = selectedNode->subEntries[i]->name;

				// first casting to a uint64 is neccesary to get rid of waring for casting to void* in next line
				
				uint64_t textureId = (uint64_t)selectedNode->subEntries[i]->thumbnailTexure_ID;
				if (!selectedNode->subEntries[i]->thumbnailLoaded) {
					textureId = 0;
				}

				bool pressed = ImGui::ImageButton((void*)textureId, button_size, ImVec2(0, 1), ImVec2(1, 0));
				//bool pressed = ImGui::ImageButton((void*)(selectedNode->subMaterials[i]->thumbnailTexure_ID), button_size, ImVec2(0, 1), ImVec2(1, 0));


				if (pressed) {

					// check if its already the active material
					if (!m_materialRegistry.IsActiveEntry(selectedNode->subEntries[i]->runtime_ID)) {

						//fs::path matDir = selectedNode->pathFromRoot / fs::path(matName);
						//m_materialRegistry.LoadActiveMaterialFromFile_Multithreaded(selectedNode->subEntries[i]);
						m_materialRegistry.LibEntry_Load(selectedNode->subEntries[i]);
					}
				}
				// Calculate size of the name and shorten it if its longer then the button size
				std::string matNameString = matName;

				while (ImGui::CalcTextSize(matNameString.c_str()).x > (m_imgButtonSize - 5.0f)) {
					matNameString.pop_back();
				};

				if (matNameString != matName) {
					matNameString += "..";
				}



#ifdef mnemosy_gui_showDebugInfo
				std::string nameWithDebugInfo = matName + " -GLTexID: " + std::to_string(selectedNode->subMaterials[i]->thumbnailTexure_ID);
				ImGui::Text(nameWithDebugInfo.c_str());
#else						
				ImGui::Text(matNameString.c_str());
#endif // mnemosy_gui_showDebugInfo

				ImGui::EndGroup();

				float last_button_x2 = ImGui::GetItemRectMax().x;
				float next_button_x2 = last_button_x2 + style.ItemSpacing.x + button_size.x;
				
				if (i + 1 < materialCount && next_button_x2 < window_visible_x2) {
					ImGui::SameLine();
				}
			}
		}


	}

	void ContentsGuiPanel::DrawMaterialButtonsOfSearch() {

		namespace fs = std::filesystem;

		ImGui::Text("Search Results:");
		ImGui::SameLine();
		ImGui::SliderFloat("Icon Size", &m_imgButtonSize, 32.0f, 350.0f, "%.0f");

		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();


		//ImGui::SeparatorText("Search Results: ");


		//ImGui::SliderFloat("Icon Size", &m_imgButtonSize, 32.0f, 350.0f, "%.0f");
		//ImGui::Spacing();


		std::vector<systems::LibEntry*>& searchResultsList = m_materialRegistry.GetSearchResultsList();

		if (!searchResultsList.empty()) {


			ImGuiStyle& style = ImGui::GetStyle();
			float window_visible_x2 = ImGui::GetWindowPos().x + ImGui::GetWindowContentRegionMax().x;
			ImVec2 button_size(m_imgButtonSize, m_imgButtonSize);

			int materialCount = searchResultsList.size();

			for (int i = 0; i < materialCount; i++) {

				ImGui::BeginGroup();

				systems::LibEntry* libEntry = searchResultsList[i];
				std::string& matName = libEntry->name;


				// first casting to a uint64 is neccesary to get rid of waring for casting to void* in next line
				uint64_t textureId = (uint64_t)libEntry->thumbnailTexure_ID;
				bool pressed = ImGui::ImageButton((void*)textureId, button_size, ImVec2(0, 1), ImVec2(1, 0));


				if (pressed) {

					// check if its already the active material
					if (!m_materialRegistry.IsActiveEntry(libEntry->runtime_ID)) {

						//::path matDir = matInfo->parent->pathFromRoot / fs::path(matName);
						//m_materialRegistry.LoadActiveMaterialFromFile_Multithreaded( matInfo);
						m_materialRegistry.LibEntry_Load(libEntry);
					}
				}

				// Calculate size of the name and shorten it if its longer then the button size
				std::string matNameString = matName;

				while (ImGui::CalcTextSize(matNameString.c_str()).x > (m_imgButtonSize - 5.0f)) {
					matNameString.pop_back();
				};

				if (matNameString != matName) {
					matNameString += "..";
				}

				ImGui::Text(matNameString.c_str());

				ImGui::EndGroup();

				float last_button_x2 = ImGui::GetItemRectMax().x;
				float next_button_x2 = last_button_x2 + style.ItemSpacing.x + button_size.x;

				if (i + 1 < materialCount && next_button_x2 < window_visible_x2) {
					ImGui::SameLine();
				}
			}
		}

	}




} // namespace end mnemosy::gui


