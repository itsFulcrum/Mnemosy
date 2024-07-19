#ifndef GUI_PANEL_H
#define GUI_PANEL_H

#include <string>



namespace mnemosy::gui
{
	
	enum GuiPanelType {
		MNSY_GUI_PANEL_NONE = 0,
		MNSY_GUI_PANEL_MAIN_MENU_BAR,
		MNSY_GUI_PANEL_VIEWPORT,
		MNSY_GUI_PANEL_MATERIAL_LIBRARY,
		MNSY_GUI_PANEL_MATERIAL_EDITOR,
		MNSY_GUI_PANEL_GLOBAL_SETTINGS,
		MNSY_GUI_PANEL_SCENE_SETTINGS,
		MNSY_GUI_PANEL_DOCUMENTATION
	};
	
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
		void Deactivate() { showPanel = false; }


		std::string GetName();
		GuiPanelType GetType();

	protected:
		bool showPanel = true;
		const char* panelName = "";
		GuiPanelType panelType = MNSY_GUI_PANEL_NONE;
	};
} // mnemosy::gui

#endif // !GUI_PANEL_H
