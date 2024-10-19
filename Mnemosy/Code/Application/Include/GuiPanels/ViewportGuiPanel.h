#ifndef VIEWPORT_GUI_PANEL_H
#define VIEWPORT_GUI_PANEL_H

#include "Include/Gui/GuiPanel.h"

#include "ImGui/imgui.h"


namespace mnemosy {
	class MnemosyEngine;
}

namespace mnemosy::gui
{

	class ViewportGuiPanel : public GuiPanel {
	public:
		ViewportGuiPanel();

		virtual void Draw() override;

	private:
		void DrawViewport();

	private:
		MnemosyEngine& m_engineInstance;

		int m_viewportPosX = 1;
		int m_viewportPosY = 1;

		ImVec2 m_windowSize = ImVec2(2.0f, 2.0f);
		ImVec2 m_windowPos  = ImVec2(2.0f, 2.0f);

		ImVec2 m_avail_size = ImVec2(2.0f, 2.0f);
		ImVec2 m_imageSize  = ImVec2(2.0f, 2.0f);
	};
}
#endif // !VIEWPORT_GUI_PANEL_H
