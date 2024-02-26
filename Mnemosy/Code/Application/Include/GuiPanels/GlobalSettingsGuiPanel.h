#ifndef GLOBAL_SETTINGS_GUI_PANEL_H
#define GLOBAL_SETTINGS_GUI_PANEL_H

#include "Include/Gui/GuiPanel.h"

namespace mnemosy::gui
{

	class GlobalSettingsGuiPanel : public GuiPanel
	{

	public:
		GlobalSettingsGuiPanel();

		virtual void Draw() override;

	};


}

#endif // !GLOBAL_SETTINGS_GUI_PANEL_H
