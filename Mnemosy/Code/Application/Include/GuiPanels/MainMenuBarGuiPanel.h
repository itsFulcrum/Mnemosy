#ifndef MAIN_MENU_BAR_GUI_PANEL_H
#define MAIN_MENU_BAR_GUI_PANEL_H

#include "Include/Gui/GuiPanel.h"


namespace mnemosy::gui {
	class GuiPanelManager;
}

namespace mnemosy::gui
{

	class MainMenuBarGuiPanel : public GuiPanel {
	public:
		MainMenuBarGuiPanel();

		virtual void Draw() override;
		
	private:
		void DataDropdown();
		void ViewsDropdown();
	private:

		const char* m_dataDropdown_ImGuiLabel = "Data";

		bool m_loadMnemosyLibraryModal = false;
		bool m_open_loadMnemosyLibraryModel = false;

		bool viewportPanel = false;
		bool sceneSettings = false;
		bool materialEditorPanel = false;
		bool materialLibraryPanel = false;
		bool documentationPanel = false;
	};
}
#endif // !MAIN_MENU_BAR_GUI_PANEL_H
