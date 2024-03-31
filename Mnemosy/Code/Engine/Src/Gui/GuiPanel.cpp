#include "Include/Gui/GuiPanel.h"

#include "Include/Core/Log.h"

#include <External/ImGui/imgui.h>

namespace mnemosy::gui
{
	GuiPanel::GuiPanel() {
		//MNEMOSY_TRACE("GuiPanel Constructor");
	}

	GuiPanel::~GuiPanel() {
		//MNEMOSY_TRACE("GuiPanel: Destructor");

	}


	void GuiPanel::Render()
	{
		Draw();
	}

	std::string GuiPanel::GetName()
	{
		return panelName;
	}

} // !mnemosy::gui