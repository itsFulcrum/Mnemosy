#include "Include/GuiPanels/MainMenuBarGuiPanel.h"

#include "Include/Application.h"
#include "Include/GuiPanels/GuiPanelManager.h"
#include "Include/GuiPanels/ViewportGuiPanel.h"
#include "Include/GuiPanels/GlobalSettingsGuiPanel.h"
#include "Include/GuiPanels/SceneSettingsGuiPanel.h"
#include "Include/GuiPanels/MaterialEditorGuiPanel.h"
#include "Include/GuiPanels/MaterialLibraryGuiPanel.h"


#include "External/ImGui/imgui.h"

namespace mnemosy::gui
{
	MainMenuBarGuiPanel::MainMenuBarGuiPanel()
	{
		panelName = "Main Menu Bar";
	}

	void MainMenuBarGuiPanel::Draw()
	{
		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("Windows"))
			{
				GuiPanelManager& panelManager = Application::GetInstance().GetGuiPanelManager();


				bool viewportPanel = panelManager.GetViewportPanel().isActive();
				if (ImGui::MenuItem("Viewport", "", viewportPanel, !viewportPanel))
				{
					panelManager.GetViewportPanel().setActive();
				}


				bool globalSettingsPanel = panelManager.GetGlobalSettingsPanel().isActive();
				if (ImGui::MenuItem("Global Settings", "",globalSettingsPanel,!globalSettingsPanel)) 
				{
					panelManager.GetGlobalSettingsPanel().setActive();
				}

				bool sceneSettings = panelManager.GetSceneSettingsPanel().isActive();
				if (ImGui::MenuItem("Scene Settings", "", sceneSettings, !sceneSettings))
				{
					panelManager.GetSceneSettingsPanel().setActive();
				}

				bool materialEditorPanel = panelManager.GetMaterialEditorPanel().isActive();
				if (ImGui::MenuItem("Material Editor", "", materialEditorPanel, !materialEditorPanel))
				{
					panelManager.GetMaterialEditorPanel().setActive();
				}
				bool materialLibraryPanel = panelManager.GetMaterialLibraryPanel().isActive();
				if (ImGui::MenuItem("Material Library", "", materialLibraryPanel, !materialLibraryPanel))
				{
					panelManager.GetMaterialLibraryPanel().setActive();
				}

				ImGui::EndMenu();
			}

			//if (ImGui::BeginMenu("Edit"))
			//{
			//	if (ImGui::MenuItem("Undo", "CTRL+Z")) {}
			//	if (ImGui::MenuItem("Redo", "CTRL+Y", false, false)) {}  // Disabled item
			//	ImGui::Separator();
			//	if (ImGui::MenuItem("Cut", "CTRL+X")) {}
			//	if (ImGui::MenuItem("Copy", "CTRL+C")) {}
			//	if (ImGui::MenuItem("Paste", "CTRL+V")) {}
			//	ImGui::EndMenu();
			//}

			ImGui::EndMainMenuBar();
		}
	}
}
