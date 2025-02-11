#ifndef LOG_GUI_PANEL_H
#define LOG_GUI_PANEL_H

#include "Include/Gui/GuiPanel.h"


namespace mnemosy::gui {

	class LogGuiPanel : public GuiPanel {
	public:
		LogGuiPanel();
		~LogGuiPanel();

		virtual void Draw() override;

	private:		

	};
}

#endif // !LOG_GUI_PANEL_H
