#ifndef MAIN_MENU_BAR_GUI_PANEL_H
#define MAIN_MENU_BAR_GUI_PANEL_H

#include "Include/Gui/GuiPanel.h"


namespace mnemosy::gui {
	class GuiPanelManager;
}

namespace mnemosy::gui
{
	class MainMenuBarGuiPanel : public GuiPanel
	{
	public:
		MainMenuBarGuiPanel();

		virtual void Draw() override;
		
	private:
		//GuiPanelManager* m_panelManager;


		bool viewportPanel = false;
		bool globalSettingsPanel = false;
		bool sceneSettings = false;
		bool materialEditorPanel = false;
		bool materialLibraryPanel = false;
		bool documentationPanel = false;
		

		
	};
}
#endif // !MAIN_MENU_BAR_GUI_PANEL_H
