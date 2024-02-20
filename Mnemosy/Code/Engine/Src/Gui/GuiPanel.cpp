#include "Engine/Include/Gui/GuiPanel.h"

#include "Engine/Include/Core/Log.h"
#include <Engine/External/ImGui/imgui.h>

namespace mnemosy::gui
{
	GuiPanel::GuiPanel()
	{
		//MNEMOSY_TRACE("GuiPanel Constructor");
	}
	void GuiPanel::Render()
	{
		Draw();
	}

	std::string GuiPanel::GetName()
	{
		return panelName;
	}

}