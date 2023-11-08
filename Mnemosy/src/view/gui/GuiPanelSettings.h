#pragma once

#include <view/gui/GuiPanel.h>


class GuiPanelSettings : public GuiPanel
{
	void Render() override
	{
		ImGui::Begin("Settings", &show);

		static float f = 0.0f;
		ImGui::SliderFloat("Environment Texture Opacity", &f, 0.0f, 1.0f);


		ImGui::End();
	}

	int Id;
};