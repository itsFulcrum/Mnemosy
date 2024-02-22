#include "Application/Include/GuiPanels/MainMenuBarGuiPanel.h"

#include "Application/Include/Application.h"
#include "Application/Include/GuiPanels/GuiPanelManager.h"

#include "Application/Include/GuiPanels/ViewportGuiPanel.h"
#include "Application/Include/GuiPanels/GlobalSettingsGuiPanel.h"
#include "Application/Include/GuiPanels/SceneSettingsGuiPanel.h"
#include "Application/Include/GuiPanels/MaterialEditorGuiPanel.h"



#include "Engine/External/ImGui/imgui.h"

namespace mnemosy::gui
{
	MainMenuBarGuiPanel::MainMenuBarGuiPanel()
	{
		panelName = "Main Menu Bar";
	}

	void MainMenuBarGuiPanel::Draw()
	{
		//if (!showPanel)
			//return;

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
