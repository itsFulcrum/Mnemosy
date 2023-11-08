#pragma once

#include <view/gui/GuiPanel.h>


class GuiPanelViewport : public GuiPanel
{
	void Render() override
	{
		ImGui::Begin("Other Viewport", &show);

		static float f = 0.0f;
		ImGui::SliderFloat("SomeViewport", &f, 0.0f, 1.0f);


		ImGui::End();
	}
};