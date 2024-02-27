#include "Include/GuiPanels/GlobalSettingsGuiPanel.h"
#include "External/ImGui/imgui.h"
#include "Include/Core/Log.h"

#include "Include/Application.h"
#include "Include/MnemosyEngine.h"
#include "Include/Core/Clock.h"
#include "Include/Graphics/Renderer.h"
#include "Include/Core/Window.h"

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


		ImGui::SeparatorText("Info");

		// show fps and frametime in ms
		MnemosyEngine& engine = ENGINE_INSTANCE();
		core::Clock& clock = engine.GetClock();

		int fps = clock.GetFPS();
		float deltaSeconds = clock.GetFrameTime();
		ImGui::Text("FPS: %d", fps);
		ImGui::Text("FrameTime: %f ms", deltaSeconds);


		// --- Render Settings
		ImGui::SeparatorText("Render Settings");

		{
			graphics::Renderer& renderer = engine.GetRenderer();
			core::Window& window = engine.GetWindow();

			const char* MSAA_Settings[5] = { "OFF","2X","4X","8X","16X"}; // they need to be ordered the same as in renderer MSAAsamples Enum
			int previewMSAA_Current = renderer.GetMSAA();
			ImGui::Combo("MSAA", &previewMSAA_Current, MSAA_Settings, IM_ARRAYSIZE(MSAA_Settings));
			if (previewMSAA_Current != renderer.GetMSAA())
			{
				renderer.SetMSAASamples((graphics::MSAAsamples)previewMSAA_Current);
			}
			ImGui::TextWrapped("MSAA OFF is not implemented at the moment and causes creashes \nSo setting It to off is same as to 2X for now");

			ImGui::Checkbox("Cap Delta Time",&clock.capDeltaTime);
			bool vSyncEnabled = window.IsVsyncEnabled();
			ImGui::Checkbox("Enable VSync",&vSyncEnabled);
			if (vSyncEnabled != window.IsVsyncEnabled())
			{
				window.EnableVsync(vSyncEnabled);
			}



		}



		ImGui::End();
	}
}