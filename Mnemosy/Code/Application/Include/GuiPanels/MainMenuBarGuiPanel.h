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

		void PopupModal_UserMessage();

	private:

		const char* m_dataDropdown_ImGuiLabel = "Data";

		//bool m_loadMnemosyLibraryModal = false;
		//bool m_open_loadMnemosyLibraryModel = false;

		// window panels visibility state
		bool m_active_viewportPanel = false;
		bool m_active_sceneSettings = false;
		bool m_active_materialEditorPanel = false;
		bool m_active_materialLibraryPanel = false;
		bool m_active_documentationPanel = false;
		bool m_active_contentsPanel = false;
		bool m_active_logPanel = false;


		bool m_popupModal_userMessage_triggered = false;

	};
}
#endif // !MAIN_MENU_BAR_GUI_PANEL_H
