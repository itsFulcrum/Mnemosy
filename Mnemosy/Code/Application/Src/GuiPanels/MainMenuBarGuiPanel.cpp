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


			ImGui::EndMenu();
		}

	}

	void MainMenuBarGuiPanel::ViewsDropdown() {

		if (ImGui::BeginMenu("Views")) {

			GuiPanelManager& m_panelManager = Application::GetInstance().GetGuiPanelManager();


			viewportPanel = m_panelManager.GetViewportPanel().isActive();
			if (ImGui::MenuItem("Viewport", "", viewportPanel, !viewportPanel)) {
				m_panelManager.GetViewportPanel().setActive();
			}

			globalSettingsPanel = m_panelManager.GetGlobalSettingsPanel().isActive();
			if (ImGui::MenuItem("Global Settings", "", globalSettingsPanel, !globalSettingsPanel)) {
				m_panelManager.GetGlobalSettingsPanel().setActive();
			}

			sceneSettings = m_panelManager.GetSceneSettingsPanel().isActive();
			if (ImGui::MenuItem("Scene Settings", "", sceneSettings, !sceneSettings)) {
				m_panelManager.GetSceneSettingsPanel().setActive();
			}

			materialEditorPanel = m_panelManager.GetMaterialEditorPanel().isActive();
			if (ImGui::MenuItem("Material Editor", "", materialEditorPanel, !materialEditorPanel)) {
				m_panelManager.GetMaterialEditorPanel().setActive();
			}

			materialLibraryPanel = m_panelManager.GetMaterialLibraryPanel().isActive();
			if (ImGui::MenuItem("Material Library", "", materialLibraryPanel, !materialLibraryPanel)) {
				m_panelManager.GetMaterialLibraryPanel().setActive();
			}

			documentationPanel = m_panelManager.GetDocumentationPanel().isActive();
			if (ImGui::MenuItem("Documentation", "", documentationPanel, !documentationPanel)) {
				m_panelManager.GetDocumentationPanel().setActive();
			}

			ImGui::EndMenu();
		} // End Menu Windows
	}
}
