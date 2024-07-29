#include "Include/GuiPanels/MainMenuBarGuiPanel.h"

#include "Include/MnemosyEngine.h"
#include "Include/Core/Log.h"
#include "Include/Systems/UserSettingsManager.h"


#include "Include/Application.h"
#include "Include/GuiPanels/GuiPanelManager.h"
#include "Include/GuiPanels/ViewportGuiPanel.h"
#include "Include/GuiPanels/SceneSettingsGuiPanel.h"
#include "Include/GuiPanels/MaterialEditorGuiPanel.h"
#include "Include/GuiPanels/MaterialLibraryGuiPanel.h"
#include "Include/GuiPanels/DocumentationGuiPanel.h"

#include "Include/Systems/MaterialLibraryRegistry.h"


#include "Include/Systems/SkyboxAssetRegistry.h"

#include "Include/Graphics/Skybox.h"
#include "Include/Graphics/RenderMesh.h"
#include "Include/Graphics/Scene.h"
#include "Include/Graphics/Renderer.h"


#include "External/ImGui/imgui.h"






namespace mnemosy::gui
{
	MainMenuBarGuiPanel::MainMenuBarGuiPanel() {

		//m_panelManager = &Application::GetInstance().GetGuiPanelManager();
		panelName = "Main Menu Bar";
		panelType = MNSY_GUI_PANEL_MAIN_MENU_BAR;
	}

	void MainMenuBarGuiPanel::Draw()
	{
		if (ImGui::BeginMainMenuBar()) {



			
			DataDropdown();
			ViewsDropdown();





			MnemosyEngine& engine = MnemosyEngine::GetInstance();

			graphics::Scene& scene = engine.GetScene();

			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::Text("														");

			// Quick select mesh
			{
				graphics::RenderMesh& renderMesh = scene.GetMesh();

				const char* previewMesh_List[7] = { "Custom","Default","Cube","Plane","Sphere","Cylinder","Suzanne" }; // they need to be ordered the same as in lightType Enum in light class

				int previewMesh_Current = (int)scene.GetCurrentPreviewMesh();


				ImGui::Text("Mesh: ");
				ImGui::SetNextItemWidth(120.0f);
				ImGui::Combo(" ##ViewportMesh", &previewMesh_Current, previewMesh_List, IM_ARRAYSIZE(previewMesh_List));

				if ((int)scene.GetCurrentPreviewMesh() != previewMesh_Current)
				{
					if (previewMesh_Current == 0)
						scene.SetPreviewMesh(graphics::PreviewMesh::Custom);
					else if (previewMesh_Current == 1)
						scene.SetPreviewMesh(graphics::PreviewMesh::Default);
					else if (previewMesh_Current == 2)
						scene.SetPreviewMesh(graphics::PreviewMesh::Cube);
					else if (previewMesh_Current == 3)
						scene.SetPreviewMesh(graphics::PreviewMesh::Plane);
					else if (previewMesh_Current == 4)
						scene.SetPreviewMesh(graphics::PreviewMesh::Sphere);
					else if (previewMesh_Current == 5)
						scene.SetPreviewMesh(graphics::PreviewMesh::Cylinder);
					else if (previewMesh_Current == 6)
						scene.SetPreviewMesh(graphics::PreviewMesh::Suzanne);
				}

			}
			ImGui::Spacing();
			ImGui::Spacing();

			// Quick Select Skybox
			{

				graphics::Skybox& skybox = scene.GetSkybox();
				mnemosy::systems::SkyboxAssetRegistry& skyboxRegistry = engine.GetSkyboxAssetRegistry();

				// -- Skybox Selection Menu
				bool assetsInRegistry = !skyboxRegistry.GetVectorOfNames().empty();
				if (assetsInRegistry) // if there are no assets in the internal vector this will crash
				{

					int current = skyboxRegistry.GetCurrentSelected();

					const char* combo_preview_value = skyboxRegistry.GetVectorOfNames()[current].c_str();
					int previousSelected = current;

					ImGui::Text("Skybox: ");
					ImGui::SetNextItemWidth(200.0f);
					if (ImGui::BeginCombo(" ##ViewportSkybox", combo_preview_value, 0))
					{
						for (int n = 0; n < skyboxRegistry.GetVectorOfNames().size(); n++)
						{
							const bool is_selected = (current == n);
							if (ImGui::Selectable(skyboxRegistry.GetVectorOfNames()[n].c_str(), is_selected))
								current = n;

							// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
							if (is_selected)
								ImGui::SetItemDefaultFocus();
						}
						ImGui::EndCombo();
					}

					if (current != previousSelected)  // Selection has happend
					{
						skybox.LoadPreviewSkybox(skyboxRegistry.GetVectorOfNames()[current]);
						engine.GetRenderer().SetShaderSkyboxUniforms(skybox);
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


				ImGui::Text("Render: ");
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

		if (ImGui::BeginMenu(m_dataDropdown_ImGuiLabel)) {

			if (ImGui::MenuItem("Save All")) {

				MnemosyEngine& engineInstance = MnemosyEngine::GetInstance();


				engineInstance.GetMaterialLibraryRegistry().SaveActiveMaterialToFile();
				engineInstance.GetMaterialLibraryRegistry().SaveUserDirectoriesData();

				engineInstance.GetUserSettingsManager().SaveToFile();
				MNEMOSY_INFO("Saved library");
			}


			if (ImGui::MenuItem("Restore Default Settings")) {

				MnemosyEngine::GetInstance().GetUserSettingsManager().LoadUserSettings(true);

			}


			ImGui::EndMenu();
		}

	}

	void MainMenuBarGuiPanel::ViewsDropdown() {

		if (ImGui::BeginMenu("Views")) {

			GuiPanelManager& m_panelManager = Application::GetInstance().GetGuiPanelManager();


			viewportPanel = m_panelManager.GetViewportPanel().IsActive();
			if (ImGui::MenuItem("Viewport", "", viewportPanel, !viewportPanel)) {
				m_panelManager.GetViewportPanel().SetActive(true);
			}

			sceneSettings = m_panelManager.GetSceneSettingsPanel().IsActive();
			if (ImGui::MenuItem("Settings", "", sceneSettings, !sceneSettings)) {
				m_panelManager.GetSceneSettingsPanel().SetActive(true);
			}

			materialEditorPanel = m_panelManager.GetMaterialEditorPanel().IsActive();
			if (ImGui::MenuItem("Material Editor", "", materialEditorPanel, !materialEditorPanel)) {
				m_panelManager.GetMaterialEditorPanel().SetActive(true);
			}

			materialLibraryPanel = m_panelManager.GetMaterialLibraryPanel().IsActive();
			if (ImGui::MenuItem("Material Library", "", materialLibraryPanel, !materialLibraryPanel)) {
				m_panelManager.GetMaterialLibraryPanel().SetActive(true);
			}

			documentationPanel = m_panelManager.GetDocumentationPanel().IsActive();
			if (ImGui::MenuItem("Documentation", "", documentationPanel, !documentationPanel)) {
				m_panelManager.GetDocumentationPanel().SetActive(true);
			}

			ImGui::EndMenu();
		} // End Menu Windows
	}
}
