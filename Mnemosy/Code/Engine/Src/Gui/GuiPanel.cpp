#include "Include/Gui/GuiPanel.h"

#include "Include/Core/Log.h"

#include <ImGui/imgui.h>

namespace mnemosy::gui
{
	GuiPanel::GuiPanel() {
	}

	GuiPanel::~GuiPanel() {

	}
	void GuiPanel::Render()
	{
		Draw();
	}

	std::string GuiPanel::GetName()
	{
		return std::string(panelName);
	}

	GuiPanelType GuiPanel::GetType()
	{
		return panelType;
	}

} // !mnemosy::gui