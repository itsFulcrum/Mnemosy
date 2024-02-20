#ifndef VIEWPORT_GUI_PANEL_H
#define VIEWPORT_GUI_PANEL_H

#include "Engine/Include/Gui/GuiPanel.h"


namespace mnemosy::gui
{
	class ViewportGuiPanel : public GuiPanel
	{

	public:
		ViewportGuiPanel();

		virtual void Draw() override;

	};


}


#endif // !VIEWPORT_GUI_PANEL_H
