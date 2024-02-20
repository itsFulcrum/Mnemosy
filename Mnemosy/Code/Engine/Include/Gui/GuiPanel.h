#ifndef GUI_PANEL_H
#define GUI_PANEL_H

#include <string>

namespace mnemosy::gui
{
	class GuiPanel
	{
	public:
		GuiPanel();
		~GuiPanel() = default;

		void Render();

		virtual void Initialize() {};
		virtual void Draw() {};


		std::string GetName();
	protected:
		bool showPanel = true;
		std::string panelName = "";

	};
}

#endif // !GUI_PANEL_H
