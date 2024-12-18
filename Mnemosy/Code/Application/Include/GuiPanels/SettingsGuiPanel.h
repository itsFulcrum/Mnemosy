#ifndef SCENE_SETTINGS_GUI_PANEL_H
#define SCENE_SETTINGS_GUI_PANEL_H

#include "Include/Gui/GuiPanel.h"

namespace mnemosy::gui
{
	class SettingsGuiPanel : public GuiPanel
	{
	public:
		SettingsGuiPanel();

		virtual void Draw() override;

	private:
				
		//bool m_openChangeDirectoryModal = false;
		//bool m_changeDirectoryModelState = false;
	};
}
#endif // !SCENE_SETTINGS_GUI_PANEL_H
