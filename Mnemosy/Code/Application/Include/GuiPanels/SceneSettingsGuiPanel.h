#ifndef SCENE_SETTINGS_GUI_PANEL_H
#define SCENE_SETTINGS_GUI_PANEL_H

#include "Include/Gui/GuiPanel.h"

namespace mnemosy::gui
{
	class SceneSettingsGuiPanel : public GuiPanel
	{
	public:
		SceneSettingsGuiPanel();

		virtual void Draw() override;

	private:
		bool m_saveSkyboxPermanentlyUponLoad = false;
		
		bool m_openChangeDirectoryModal = false;
		bool m_changeDirectoryModelState = false;
	};
}
#endif // !SCENE_SETTINGS_GUI_PANEL_H
