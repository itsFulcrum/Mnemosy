#ifndef MAIN_MENU_BAR_GUI_PANEL_H
#define MAIN_MENU_BAR_GUI_PANEL_H

#include "Include/Gui/GuiPanel.h"

namespace mnemosy::gui
{
	class MainMenuBarGuiPanel : public GuiPanel
	{
	public:
		MainMenuBarGuiPanel();

		virtual void Draw() override;
	};
}
#endif // !MAIN_MENU_BAR_GUI_PANEL_H
