#include "Include/GuiPanels/GlobalSettingsGuiPanel.h"
#include "External/ImGui/imgui.h"
#include "Include/Core/Log.h"

#include "Include/Application.h"
#include "Include/MnemosyEngine.h"
#include "Include/Core/Clock.h"
#include "Include/Graphics/Renderer.h"

namespace mnemosy::gui
{
	GlobalSettingsGuiPanel::GlobalSettingsGuiPanel()
	{
		panelName = "Global Settings";
	}

	void GlobalSettingsGuiPanel::Draw()
	{
		if (!showPanel)
			return;

		ImGui::Begin(panelName.c_str(), &showPanel);

		ImGui::Text("Here will go global settings");

		// show fps and frametime in ms
		MnemosyEngine& engine = ENGINE_INSTANCE();
		core::Clock& clock = engine.GetClock();

		int fps = clock.GetFPS();
		float deltaSeconds = clock.GetFrameTime();
		ImGui::Text("FPS: %d", fps);
		ImGui::Text("FrameTime: %f ms", deltaSeconds);

		ImGui::SeparatorText("Render Settings");

		{
			graphics::Renderer& renderer = engine.GetRenderer();

			ImGui::TextWrapped("MSAA OFF is not implemented at the moment and causes creashes\n So setting It to off is equvalent to MSAA2X for now");

			const char* MSAA_Settings[5] = { "OFF","2X","4X","8X","16X"}; // they need to be ordered the same as in lightType Enum in light class
			int previewMSAA_Current = renderer.GetMSAA();
			ImGui::Combo("MSAA", &previewMSAA_Current, MSAA_Settings, IM_ARRAYSIZE(MSAA_Settings));
			if (previewMSAA_Current != renderer.GetMSAA())
			{
				if (previewMSAA_Current == 0)
					renderer.SetMSAASamples(graphics::MSAAsamples::MSAAOFF);
				else if (previewMSAA_Current == 1)
					renderer.SetMSAASamples(graphics::MSAAsamples::MSAA2X);
				else if (previewMSAA_Current == 2)
					renderer.SetMSAASamples(graphics::MSAAsamples::MSAA4X);
				else if (previewMSAA_Current == 3)
					renderer.SetMSAASamples(graphics::MSAAsamples::MSAA8X);
				else if (previewMSAA_Current == 4)
					renderer.SetMSAASamples(graphics::MSAAsamples::MSAA16X);
			}


		}



		ImGui::End();
	}
}