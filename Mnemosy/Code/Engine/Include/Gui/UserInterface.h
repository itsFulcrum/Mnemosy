#ifndef USER_INTERFACE_H
#define USER_INTERFACE_H

#include <vector>

#include <GLFW/glfw3.h>

#include <Engine/External/ImGui/imgui.h>
#include <Engine/External/ImGui/imgui_impl_glfw.h>
#include <Engine/External/ImGui/imgui_impl_opengl3.h>



#include "Engine/Include/Gui/GuiPanel.h"


namespace mnemosy::gui
{
	class UserInterface
	{
	public:
		UserInterface();
		~UserInterface();

		void Render();

		bool WantCaptureInput();

		void RegisterGuiPanel(GuiPanel* newPanel);
		void UnregisterGuiPanel(GuiPanel* existingPanel);


		bool show_demo_window = true;
	private:
		void SetupImGuiStyle();
		void StartFrame();
		void EndFrame();


		std::vector<GuiPanel*> m_guiPanels;

		ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoScrollbar;
	};


}


#endif // !USER_INTERFACE_H
