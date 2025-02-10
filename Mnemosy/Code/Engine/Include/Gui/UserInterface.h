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
		UserInterface()  = default;
		~UserInterface() = default;

		void Init();
		void Shutdown();

		void Render();

		bool WantCaptureInput();

		void RegisterGuiPanel(GuiPanel* newPanel);
		void UnregisterGuiPanel(GuiPanel* existingPanel);

		void RegisterMainMenuBarGuiPanel(GuiPanel& newMenuBarPanel);
		void UnregisterMainMenuBarGuiPanel();

		bool IsGuiPanelVisible(GuiPanelType panelType);
		GuiPanel& GetGuiPanel(GuiPanelType panelType);
		
		ImFont* Font_Text_Get() { return m_font_text; }
		ImFont* Font_Icon_Get() { return m_font_icon; }

		bool show_demo_window = false;
	private:
		void SetupImGuiStyle();
		void StartFrame();
		void EndFrame();



		GuiPanel* m_mainMenuBarGuiPanel = nullptr;
		std::vector<GuiPanel*> m_guiPanels;
		ImGuiWindowFlags m_window_flags = ImGuiWindowFlags_NoScrollbar;

		ImFont* m_font_text = nullptr;
		ImFont* m_font_icon = nullptr;

		ImVector<ImWchar> m_font_text_glyph_ranges;
		ImVector<ImWchar> m_font_icons_glyph_ranges;
	
	};

} // mnemosy::gui

#endif // !USER_INTERFACE_H
