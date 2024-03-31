#ifndef GUI_PANEL_H
#define GUI_PANEL_H

#include <string>

namespace mnemosy::gui
{
	class GuiPanel
	{
	public:
		GuiPanel();
		~GuiPanel();

		void Render();

		virtual void Initialize() {};
		virtual void Draw() {};

		bool isActive() { return showPanel; }
		void setActive() { showPanel = true; }

		std::string GetName();
	protected:
		bool showPanel = true;
		std::string panelName = "";

	};
} // mnemosy::gui

#endif // !GUI_PANEL_H
