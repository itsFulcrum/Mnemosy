#ifndef USER_INTERFACE_H
#define USER_INTERFACE_H

#include "Include/Gui/GuiPanel.h"


#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_glfw.h"
#include "ImGui/imgui_impl_opengl3.h"

#include <vector>
#include <GLFW/glfw3.h>

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

		void RegisterMainMenuBarGuiPanel(GuiPanel& newMenuBarPanel);
		void UnregisterMainMenuBarGuiPanel();

		bool IsGuiPanelVisible(GuiPanelType panelType);
		GuiPanel& GetGuiPanel(GuiPanelType panelType);

		bool show_demo_window = false;
	private:
		void SetupImGuiStyle();
		void StartFrame();
		void EndFrame();



		GuiPanel* m_mainMenuBarGuiPanel = nullptr;
		std::vector<GuiPanel*> m_guiPanels;
		ImGuiWindowFlags m_window_flags = ImGuiWindowFlags_NoScrollbar;
	
	};

} // mnemosy::gui

#endif // !USER_INTERFACE_H
