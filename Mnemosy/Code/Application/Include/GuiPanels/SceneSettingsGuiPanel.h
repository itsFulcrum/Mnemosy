#ifndef SCENE_SETTINGS_GUI_PANEL_H
#define SCENE_SETTINGS_GUI_PANEL_H


#include "Engine/Include/Gui/GuiPanel.h"


namespace mnemosy::gui
{
	class SceneSettingsGuiPanel : public GuiPanel
	{

	public:
		SceneSettingsGuiPanel();

		virtual void Draw() override;

	};


}

#endif // !SCENE_SETTINGS_GUI_PANEL_H
