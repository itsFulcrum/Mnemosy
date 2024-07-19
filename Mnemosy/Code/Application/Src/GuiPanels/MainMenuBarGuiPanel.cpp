#include "Include/GuiPanels/MainMenuBarGuiPanel.h"

#include "Include/MnemosyEngine.h"
#include "Include/Systems/UserSettingsManager.h"


#include "Include/Application.h"
#include "Include/GuiPanels/GuiPanelManager.h"
#include "Include/GuiPanels/ViewportGuiPanel.h"
#include "Include/GuiPanels/GlobalSettingsGuiPanel.h"
#include "Include/GuiPanels/SceneSettingsGuiPanel.h"
#include "Include/GuiPanels/MaterialEditorGuiPanel.h"
#include "Include/GuiPanels/MaterialLibraryGuiPanel.h"
#include "Include/GuiPanels/DocumentationGuiPanel.h"

#include "Include/Systems/MaterialLibraryRegistry.h"

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

			globalSettingsPanel = m_panelManager.GetGlobalSettingsPanel().IsActive();
			if (ImGui::MenuItem("Global Settings", "", globalSettingsPanel, !globalSettingsPanel)) {
				m_panelManager.GetGlobalSettingsPanel().SetActive(true);
			}

			sceneSettings = m_panelManager.GetSceneSettingsPanel().IsActive();
			if (ImGui::MenuItem("Scene Settings", "", sceneSettings, !sceneSettings)) {
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
