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
#include "Include/Systems/MaterialLibraryRegistry.h"

#ifdef MNEMOSY_PLATFORM_WINDOWS
#include "Include/Core/Utils/PlatfromUtils_Windows.h"
#endif // MNEMOSY_PLATFORM_WINDOWS


#include <filesystem>

namespace mnemosy::gui
{
	GlobalSettingsGuiPanel::GlobalSettingsGuiPanel()
	{
		panelName = "Global Settings";
		panelType = MNSY_GUI_PANEL_GLOBAL_SETTINGS;
	}

	void GlobalSettingsGuiPanel::Draw()
	{
		if (!showPanel)
			return;


		MnemosyEngine& engine = ENGINE_INSTANCE();

		ImGui::Begin(panelName, &showPanel);


		// Library Directory
		{
			ImGui::SeparatorText("Library Directory");
			core::FileDirectories& fd = engine.GetFileDirectories();


			std::string currentLibraryDirectory = fd.GetLibraryDirectoryPath().generic_string();
			ImGui::Text("Path: %s", currentLibraryDirectory.c_str());

			if (ImGui::Button("Select Folder...")) {
				
				// check if the current library folder contains any data
				if (fd.ContainsUserData()) {
					// open popup modal..
					m_openChangeDirectoryModal = true;
				}
				else { // user directory does not contain anything so we can savely set a new path
#ifdef MNEMOSY_PLATFORM_WINDOWS
					std::string directoryPath = mnemosy::core::FileDialogs::SelectFolder("");
					if (!directoryPath.empty()) {
						
						
						fd.SetNewUserLibraryDirectory(std::filesystem::directory_entry(directoryPath),false,true);




					}
					else {
						MNEMOSY_ERROR("You didnt select a valid folder path");
					}
#endif
				}
				
			}
			//static bool popModal = false;


			if (m_openChangeDirectoryModal) {
							
				m_openChangeDirectoryModal = false; // to make sure its only called once
				m_changeDirectoryModelState = true;
				ImGui::OpenPopup("Change Library Directory");
			}


			if (ImGui::BeginPopupModal("Change Library Directory", &m_changeDirectoryModelState, ImGuiWindowFlags_AlwaysAutoResize)) {

				ImGui::Text("The current library directory has some files in it. \nDo you want to copy all contents over to the new directory? ");

				ImGui::Spacing();

				if (ImGui::Button("No Delete All!", ImVec2(200, 0))) {

#ifdef MNEMOSY_PLATFORM_WINDOWS
					std::string directoryPath = mnemosy::core::FileDialogs::SelectFolder("");
					if (!directoryPath.empty()) {
						fd.SetNewUserLibraryDirectory(std::filesystem::directory_entry(directoryPath), false, true);
						// delete all material and directory data from material registry
						systems::MaterialLibraryRegistry& registry = MnemosyEngine::GetInstance().GetMaterialLibraryRegistry();
						registry.ClearUserMaterialsAndFolders();

					} else {
						MNEMOSY_ERROR("You didnt select a valid folder path");
					}
#endif

					m_changeDirectoryModelState = false;
					ImGui::CloseCurrentPopup();
				}
				ImGui::SameLine();

				if (ImGui::Button("Yes Copy!", ImVec2(200, 0))) {

#ifdef MNEMOSY_PLATFORM_WINDOWS
					std::string directoryPath = mnemosy::core::FileDialogs::SelectFolder("");
					if (!directoryPath.empty()) {
						fd.SetNewUserLibraryDirectory(std::filesystem::directory_entry(directoryPath), true, true);
					}
					else {
						MNEMOSY_ERROR("You didnt select a valid folder path");
					}
#endif

					m_changeDirectoryModelState = false;
					ImGui::CloseCurrentPopup();
				}


				ImGui::Spacing();

				if (ImGui::Button("Cancel", ImVec2(150, 0))) {
					
					m_changeDirectoryModelState = false;
					ImGui::CloseCurrentPopup();
				}

				ImGui::EndPopup();
			}



		}



#ifdef mnemosy_gui_showDebugInfo


		ImGui::SeparatorText("Debug Info");

		// show fps and frametime in ms
		int fps = clock.GetFPS();
		float deltaSeconds = clock.GetFrameTime();
		ImGui::Text("FPS: %d", fps);
		ImGui::Text("FrameTime: %f ms", deltaSeconds);

#endif // mnemosy_gui_showDebugInfo

		// --- Render Settings
		ImGui::SeparatorText("Render Settings");

		{
			graphics::Renderer& renderer = engine.GetRenderer();
			core::Window& window = engine.GetWindow();
			core::Clock& clock = engine.GetClock();

			const char* MSAA_Settings[5] = { "OFF","2X","4X","8X","16X"}; // they need to be ordered the same as in renderer MSAAsamples Enum
			int previewMSAA_Current = renderer.GetMSAA();
			ImGui::Combo("MSAA", &previewMSAA_Current, MSAA_Settings, IM_ARRAYSIZE(MSAA_Settings));
			if (previewMSAA_Current != renderer.GetMSAA())
			{
				renderer.SetMSAASamples((graphics::MSAAsamples)previewMSAA_Current);
			}

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