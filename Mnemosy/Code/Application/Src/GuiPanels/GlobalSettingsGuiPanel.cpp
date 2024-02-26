#include "Include/GuiPanels/GlobalSettingsGuiPanel.h"
#include "External/ImGui/imgui.h"

namespace mnemosy::gui
{
	GlobalSettingsGuiPanel::GlobalSettingsGuiPanel()
	{
		panelName = "Global Settings";
	}

	void GlobalSettingsGuiPanel::Draw()
	{
		if (!showPanel)
			return;

		ImGui::Begin(panelName.c_str(), &showPanel);

		ImGui::Text("Here will go global settings");


		ImGui::End();
	}
}