#include "Include/GuiPanels/GlobalSettingsGuiPanel.h"
#include "External/ImGui/imgui.h"
#include "Include/Core/Log.h"

#include "Include/MnemosyConfig.h"
#include "Include/Application.h"
#include "Include/MnemosyEngine.h"
#include "Include/Core/FileDirectories.h"
#include "Include/Core/Clock.h"
#include "Include/Graphics/Renderer.h"
#include "Include/Core/Window.h"


#ifdef MNEMOSY_PLATFORM_WINDOWS
#include "Include/Core/Utils/PlatfromUtils_Windows.h"
#endif // MNEMOSY_PLATFORM_WINDOWS


#include <filesystem>

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


		MnemosyEngine& engine = ENGINE_INSTANCE();

		ImGui::Begin(panelName.c_str(), &showPanel);



		{
			ImGui::SeparatorText("Library Directory");
			core::FileDirectories& fd = engine.GetFileDirectories();


			std::string currentLibraryDirectory = fd.GetLibraryDirectoryPath().generic_string();
			ImGui::Text("Path: %s", currentLibraryDirectory.c_str());

			if (ImGui::Button("Select Folder...")) {

#ifdef MNEMOSY_PLATFORM_WINDOWS
				//std::string filepath = mnemosy::core::FileDialogs::OpenFile("All files (*.*)\0*.*\0 hdr (*.hdr)\0*.hdr\0 png (*.png)\0*.png\0 jpg (*.jpg)\0*.jpg\0");
				std::string directoryPath = mnemosy::core::FileDialogs::SelectFolder("");
				
				if (!directoryPath.empty()) {

					fd.SetUserLibraryDirectory(std::filesystem::directory_entry(directoryPath));
					
				}

#endif

			}

		}




		ImGui::SeparatorText("Info");

		// show fps and frametime in ms
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