#include "Include/GuiPanels/MainMenuBarGuiPanel.h"

#include "Include/Application.h"


#include "Include/MnemosyEngine.h"
#include "Include/MnemosyConfig.h"
#include "Include/Core/Log.h"
#include "Include/Core/Logger.h"
#include "Include/Core/FileDirectories.h"

#ifdef MNEMOSY_PLATFORM_WINDOWS
#include "Include/Core/Utils/PlatfromUtils_Windows.h"
#endif // MNEMOSY_PLATFORM_WINDOWS

#include "Include/GuiPanels/GuiPanelManager.h"
#include "Include/GuiPanels/ViewportGuiPanel.h"
#include "Include/GuiPanels/SettingsGuiPanel.h"
#include "Include/GuiPanels/MaterialEditorGuiPanel.h"
#include "Include/GuiPanels/MaterialLibraryGuiPanel.h"
#include "Include/GuiPanels/DocumentationGuiPanel.h"
#include "Include/GuiPanels/ContentsGuiPanel.h"
#include "Include/GuiPanels/LogGuiPanel.h"

#include "Include/Systems/MaterialLibraryRegistry.h"
#include "Include/Systems/SkyboxAssetRegistry.h"
#include "Include/Systems/FolderTreeNode.h"
#include "Include/Gui/UserInterface.h"

#include "Include/Graphics/Skybox.h"
#include "Include/Graphics/RenderMesh.h"
#include "Include/Graphics/Scene.h"
#include "Include/Graphics/Renderer.h"


#include "ImGui/imgui.h"


#include <string>
#include <filesystem>

namespace mnemosy::gui
{
	MainMenuBarGuiPanel::MainMenuBarGuiPanel() {

		panelName = "Main Menu Bar";
		panelType = MNSY_GUI_PANEL_MAIN_MENU_BAR;
	}

	void MainMenuBarGuiPanel::Draw()
	{
		if (ImGui::BeginMainMenuBar()) {

			PopupModal_UserMessage();

			DataDropdown();
			ViewsDropdown();

			MnemosyEngine& engine = MnemosyEngine::GetInstance();

			graphics::Scene& scene = engine.GetScene();

			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::Text("		"); // hack to get some spacing

			// Quick select mesh
			{
				graphics::RenderMesh& renderMesh = scene.GetMesh();

				const char* previewMesh_List[8] = { "Custom","Default","Cube","Plane","Sphere","Cylinder","Suzanne","Fabric" }; // they need to be ordered the same as in lightType Enum in light class

				int previewMesh_Current = (int)scene.GetCurrentPreviewMesh();


				ImGui::Text("Mesh: ");
				ImGui::SetNextItemWidth(120.0f);
				ImGui::Combo(" ##ViewportMesh", &previewMesh_Current, previewMesh_List, IM_ARRAYSIZE(previewMesh_List));

				if ((int)scene.GetCurrentPreviewMesh() != previewMesh_Current) {

					scene.SetPreviewMesh((graphics::PreviewMesh)previewMesh_Current);
				}

			}
			ImGui::Spacing();
			ImGui::Spacing();

			
			// Quick Select Skybox
			{

				graphics::Skybox& skybox = scene.GetSkybox();
				mnemosy::systems::SkyboxAssetRegistry& skyReg = engine.GetSkyboxAssetRegistry();

				// -- Skybox Selection Menu				
				const std::vector<std::string>& list = skyReg.GetEntryList();
				
				if (!list.empty())
				{
					uint16_t current_id = skyReg.GetCurrentSelectedID();
					const char* combo_preview_value = list[current_id].c_str();

					ImGui::Text("Skybox: ");
					ImGui::SetNextItemWidth(200.0f);
					if (ImGui::BeginCombo(" ##ViewportSkybox", combo_preview_value, 0))
					{
						for (uint16_t n = 0; n < list.size(); n++)
						{
							const bool is_selected = (current_id == n);
							if (ImGui::Selectable(list[n].c_str(), is_selected)) {
								current_id = n;
							}

							// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
							if (is_selected)
								ImGui::SetItemDefaultFocus();
						}
						ImGui::EndCombo();
					}

					if (current_id != skyReg.GetCurrentSelectedID())  // Selection has happend
					{

						if (MnemosyEngine::GetInstance().GetMaterialLibraryRegistry().UserEntrySelected()) {
							if (MnemosyEngine::GetInstance().GetMaterialLibraryRegistry().ActiveLibEntry_Get()->type == systems::LibEntryType::MNSY_ENTRY_TYPE_SKYBOX) {

								// if the active libEntry is of type skybox we must select default material because when switching preview skybox because 
								// otherwise it messes up thumbnail rendering of the libEntry when switching to a new one and it also overrides material data 
								// since when saving we grab the data from the aktivly loaded skybox which has changed by choosing a new preview skybox. 
								 
								MnemosyEngine::GetInstance().GetMaterialLibraryRegistry().SetDefaultMaterial();

							}
						}
						
						scene.SetSkybox(skyReg.LoadPreviewSkybox(current_id,true));
						engine.GetRenderer().SetShaderSkyboxUniforms(scene.userSceneSettings, scene.GetSkybox());
					}
				}


			}

			ImGui::Spacing();
			ImGui::Spacing();

			// Rendering Selection
			{

				graphics::Renderer& renderer = engine.GetRenderer();

				const char* renderModes_List[9] = { "Shaded","Albedo","Roughness","Metallic","Normal","AmbientOcclusion","Emissive","Height", "Opacity" }; // they need to be ordered the same as in RenderModes Enum in renderer class
				int rendermode_Current = renderer.GetCurrentRenderModeInt();


				ImGui::Text("PBR Render Mode: ");
				ImGui::SetNextItemWidth(180.0f);
				ImGui::Combo(" ##ViewportRenderModes", &rendermode_Current, renderModes_List, IM_ARRAYSIZE(renderModes_List));



				if (rendermode_Current != renderer.GetCurrentRenderModeInt())
				{
					renderer.SetRenderMode((graphics::RenderModes)rendermode_Current);
				}

			}


			ImGui::EndMainMenuBar();
		} // End ImGui::BeginMainMenuBar()

	} // End Draw()



	void MainMenuBarGuiPanel::DataDropdown() {

		namespace fs = std::filesystem;

		if (ImGui::BeginMenu(m_dataDropdown_ImGuiLabel)) {

			if (ImGui::MenuItem("Save All")) {

				
				MnemosyEngine::GetInstance().GetMaterialLibraryRegistry().SaveCurrentSate();
				Application::GetInstance().GetGuiPanelManager().UserSettingsSave();

				MNEMOSY_INFO("Saved library");
			}

			ImGui::EndMenu();
		}

	}

	void MainMenuBarGuiPanel::ViewsDropdown() {

		if (ImGui::BeginMenu("Views")) {

			GuiPanelManager& m_panelManager = Application::GetInstance().GetGuiPanelManager();


			m_active_viewportPanel = m_panelManager.GetViewportPanel().IsActive();
			if (ImGui::MenuItem("Viewport", "", m_active_viewportPanel, !m_active_viewportPanel)) {
				m_panelManager.GetViewportPanel().SetActive(true);
			}

			m_active_sceneSettings = m_panelManager.GetSettingsPanel().IsActive();
			if (ImGui::MenuItem("Settings", "", m_active_sceneSettings, !m_active_sceneSettings)) {
				m_panelManager.GetSettingsPanel().SetActive(true);
			}

			m_active_materialEditorPanel = m_panelManager.GetMaterialEditorPanel().IsActive();
			if (ImGui::MenuItem("Material Editor", "", m_active_materialEditorPanel, !m_active_materialEditorPanel)) {
				m_panelManager.GetMaterialEditorPanel().SetActive(true);
			}

			m_active_materialLibraryPanel = m_panelManager.GetMaterialLibraryPanel().IsActive();
			if (ImGui::MenuItem("Material Library", "", m_active_materialLibraryPanel, !m_active_materialLibraryPanel)) {
				m_panelManager.GetMaterialLibraryPanel().SetActive(true);
			}

			m_active_documentationPanel = m_panelManager.GetDocumentationPanel().IsActive();
			if (ImGui::MenuItem("Documentation", "", m_active_documentationPanel, !m_active_documentationPanel)) {
				m_panelManager.GetDocumentationPanel().SetActive(true);
			}

			m_active_contentsPanel = m_panelManager.GetContentsPanel().IsActive();
			if (ImGui::MenuItem("Contents", "", m_active_contentsPanel, !m_active_contentsPanel)) {
				m_panelManager.GetContentsPanel().SetActive(true);
			}

			m_active_logPanel = m_panelManager.GetLogPanel().IsActive();
			if (ImGui::MenuItem("Log", "", m_active_logPanel, !m_active_logPanel)) {
				m_panelManager.GetLogPanel().SetActive(true);
			}

			ImGui::EndMenu();
		} // End Menu Windows
	}


	void MainMenuBarGuiPanel::PopupModal_UserMessage() {

		static bool popModal_userMessage_show = false;

		core::Logger& logger = MnemosyEngine::GetInstance().GetLogger();
		if (logger.IsPopupMessageTriggered()) {

			popModal_userMessage_show = true;
			logger.UntriggerPopupMessage(); // to make sure its only called once
			ImGui::OpenPopup("Message Popup", ImGuiPopupFlags_::ImGuiPopupFlags_AnyPopup);
		}

		if (ImGui::BeginPopupModal("Message Popup", &popModal_userMessage_show, ImGuiWindowFlags_AlwaysAutoResize)) {

			ImGui::Spacing();
			ImGui::Text("Warning - Something went wrong!");
			ImGui::Spacing();

			ImGui::Text(logger.GetPopupMessage().c_str());

			ImGui::Spacing();
			ImGui::Spacing();

			if (ImGui::Button("OK")) {

				popModal_userMessage_show = false;
				ImGui::CloseCurrentPopup();
			}

			ImGui::EndPopup();
		}

	}
}
