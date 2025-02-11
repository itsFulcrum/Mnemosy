#include "Include/Gui/UserInterface.h"
#include "Include/MnemosyEngine.h"
#include "Include/MnemosyConfig.h"
#include "Include/Core/FileDirectories.h"


#include "Include/Core/Log.h"
#include "Include/Core/Window.h"

#include <filesystem>

namespace mnemosy::gui
{
	void UserInterface::Init() {

		m_mainMenuBarGuiPanel = nullptr;
		m_window_flags = ImGuiWindowFlags_NoScrollbar;


		// ImGui Setup
		// 
		// GL 4.5 + GLSL 130
		const char* glsl_version = "#version 450";

		IMGUI_CHECKVERSION();
		ImGui::CreateContext();

		ImGuiIO& io = ImGui::GetIO(); (void)io;

		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;	  // Enable Gamepad Controls
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
		//io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;     // Enable Multi-Viewport / Platform Windows


		SetupImGuiStyle();


		//MnemosyEngine::GetInstance().GetWindow().GetWindow();
		// Setup Platform/Renderer backends
		GLFWwindow* current = glfwGetCurrentContext();
		//ImGui_ImplGlfw_InitForOpenGL(&MnemosyEngine::GetInstance().GetWindow().GetWindow(), true);
		ImGui_ImplGlfw_InitForOpenGL(current, true);
		ImGui_ImplOpenGL3_Init(glsl_version);

	}

	void UserInterface::Shutdown() {

		m_guiPanels.clear();

		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();

		m_font_text_glyph_ranges.clear();
		m_font_icons_glyph_ranges.clear();

		if (m_font_text) {
			//delete m_font_text;
			m_font_text = nullptr;
		}

		if (m_font_icon) {
			//delete m_font_icon;
			m_font_icon = nullptr;
		}
	}

	void UserInterface::Render() {
		StartFrame();

		ImGui::DockSpaceOverViewport();
		ImGuiIO& io = ImGui::GetIO();


		if (m_mainMenuBarGuiPanel)
		{
			m_mainMenuBarGuiPanel->Draw();
		}

#ifdef MNEMOSY_CONFIG_DEBUG
		//show_demo_window = true;

		if (show_demo_window) {
			ImGui::ShowDemoWindow(&show_demo_window);
		}
#endif // MNEMOSY_CONFIG_DEBUG

		for (GuiPanel* panels : m_guiPanels)
		{
			panels->Render();
		}

		ImGui::Render();
		EndFrame();
	}
	bool UserInterface::WantCaptureInput()
	{
		ImGuiIO& io = ImGui::GetIO();

		return io.WantCaptureMouse || io.WantCaptureKeyboard;
		//return false;
	}
	void UserInterface::RegisterGuiPanel(GuiPanel* newPanel)
	{
		// check if panel already exists
		for (GuiPanel* panel : m_guiPanels)
		{
			if (panel->GetName() == newPanel->GetName())
			{
				// dont add it 
				MNEMOSY_ERROR("UserInterface::RegisterGuiPanel:  A gui panel with the same name already exists");
				return;
			}

			if (panel->GetType() == newPanel->GetType()) {

				// dont add it 
				MNEMOSY_ERROR("UserInterface::RegisterGuiPanel:  A gui panel with the same type already exists");
				return;
			}

		}

		m_guiPanels.push_back(newPanel);
	}
	void UserInterface::UnregisterGuiPanel(GuiPanel* existingPanel)
	{

		for (int i = 0; i < m_guiPanels.size(); i++)
		{
			if (m_guiPanels[i]->GetName() == existingPanel->GetName())
			{
				m_guiPanels.erase(m_guiPanels.begin() + i);
				return;
			}
		}
	}

	void UserInterface::RegisterMainMenuBarGuiPanel(GuiPanel& newMenuBarPanel)
	{
		MNEMOSY_ASSERT(!m_mainMenuBarGuiPanel, "There can only be one main menu bar, it was already registered");

		if (m_mainMenuBarGuiPanel)
		{
			return;
		}

		m_mainMenuBarGuiPanel = &newMenuBarPanel;
	}

	void UserInterface::UnregisterMainMenuBarGuiPanel()
	{
		MNEMOSY_ASSERT(m_mainMenuBarGuiPanel, "There is no menu bar registered yet");

		if (!m_mainMenuBarGuiPanel)
		{
			return;
		}

		//delete m_mainMenuBarGuiPanel;
		m_mainMenuBarGuiPanel = nullptr;
	}

	bool UserInterface::IsGuiPanelVisible(GuiPanelType panelType) {

		MNEMOSY_ASSERT(panelType != MNSY_GUI_PANEL_NONE, " Cannot call this with type none");
		MNEMOSY_ASSERT(!m_guiPanels.empty(), "Gui Panels should not be empty!");


		for (size_t i = 0; i < m_guiPanels.size();i++) {

			if (m_guiPanels[i]->GetType() == panelType) {

				return m_guiPanels[i]->IsActive();
			}


		}


		return false;
	}

	GuiPanel& UserInterface::GetGuiPanel(GuiPanelType panelType) {

		MNEMOSY_ASSERT(panelType != MNSY_GUI_PANEL_NONE, " Cannot call this with type none");
		MNEMOSY_ASSERT(!m_guiPanels.empty(), "Gui Panels should not be empty!");


		for (size_t i = 0; i < m_guiPanels.size(); i++) {

			if (m_guiPanels[i]->GetType() == panelType) {

				return *m_guiPanels[i];
			}


		}

		// in case we reach here there must be an error
		MNEMOSY_ASSERT(false, "This should not happen there should be a gui panel for each type");

		GuiPanel* p = nullptr;
		return *p;
	}


	// private 

	void UserInterface::SetupImGuiStyle()
	{
		ImGui::StyleColorsDark();
		ImGuiIO& io = ImGui::GetIO();
		

		namespace fs = std::filesystem;

		fs::path fontsPath = MnemosyEngine::GetInstance().GetFileDirectories().GetFontsPath();

		fs::path textFont_path = fontsPath / fs::path("RobotoRegular.ttf");
		fs::path iconFont_path = fontsPath / fs::path("MaterialSymbolsRounded_Filled_28pt-Regular.ttf");

		// ===== Load Text Font
		
		{

			// setup glyph ranges. by default not all glypths within a font are actually rendert to an altas which makes sense.
			// robot supports quite a bunch of languages but sady not symbols like cool arrows and stuff...
			ImFontGlyphRangesBuilder builder;
			builder.AddRanges(io.Fonts->GetGlyphRangesDefault());
			builder.AddRanges(io.Fonts->GetGlyphRangesCyrillic());
			builder.AddRanges(io.Fonts->GetGlyphRangesGreek());
		
			static const ImWchar ranges_turkischSpanishFrenchStuff[] = {
				0x0100, 0x03D6, 
				0,
			};

			builder.AddRanges(ranges_turkischSpanishFrenchStuff);
			builder.BuildRanges(&m_font_text_glyph_ranges);

			m_font_text = io.Fonts->AddFontFromFileTTF(textFont_path.generic_string().c_str(), 20.0f, nullptr, m_font_text_glyph_ranges.Data);
		
			if (!m_font_text) {
				MNEMOSY_ERROR("Failed to load text font");
			}

		}

		// ===== Load Icons Font
		{

			ImFontGlyphRangesBuilder builder;
			builder.AddText("abcdefghijklmnopqrstufvxyz0123456789 _ABCDEFGHIJKLMNOPQRSTUVWXYZ");
			//builder.AddChar(0xE92B); // Icon: Delete Forever
			builder.AddChar(0xE88E); // Icon: Info
			//builder.AddChar(0xE000); // Icon: Error
			builder.AddChar(0xE887); // Icon: Help
			//builder.AddChar(0xE8B9); // Icon: Settings
			//builder.AddChar(0xE14c); // Icon: Close (x)
			//builder.AddChar(0xE580); // Icon: Close small (x)
			builder.AddChar(0xE5F2); // Icon: Arrow Up Bold
			//builder.AddChar(0xE984); // Icon: Arrow Downward alt
			//builder.AddChar(0xE986); // Icon: Arrow Upward alt
			//builder.AddChar(0xEF7D); // Icon: Arrow Left alt
			//builder.AddChar(0xE251); // Icon: Image
			//builder.AddChar(0xE2C7); // Icon: Folder
			//builder.AddChar(0xE2C8); // Icon: Folder Open
			//builder.AddChar(0xE2CC); // Icon: Create New Folder



			builder.BuildRanges(&m_font_icons_glyph_ranges);


			m_font_icon = io.Fonts->AddFontFromFileTTF(iconFont_path.generic_string().c_str(), 20.0f, nullptr, m_font_icons_glyph_ranges.Data);


			if (!m_font_icon) {
				MNEMOSY_ERROR("Failed to load icons font");
			}
		}


		

		// "MaterialSymbolsRounded_Filled_28pt-Regular.ttf"

		
			
		io.Fonts->Build(); // force building the font atlas

		// Test if specific glypth exsits
		// may use a tool like this to check a font: https://fontdrop.info/?darkmode=true
		//ImWchar w = 0x02C6;
		//const ImFontGlyph* glyph = font->FindGlyphNoFallback(w);
		//if (glyph) {
		//	MNEMOSY_WARN("Found glypth ");
		//}

		// ===== Setup Syle

		ImGuiStyle& style = ImGui::GetStyle();

		style.FrameRounding = 12;
		style.WindowRounding = 12;
		style.GrabRounding = 12;
		style.DockingSeparatorSize = 2;
		style.SeparatorTextPadding.x = 6;

		//ImVec4* colors = ImGui::GetStyle().Colors;
		//colors[ImGuiCol_WindowBg] = ImVec4(0.44f, 0.26f, 0.26f, 0.94f);
		ImVec4* colors = ImGui::GetStyle().Colors;
		colors[ImGuiCol_Text]				= ImVec4(0.68f, 0.68f, 0.68f, 1.00f);
		colors[ImGuiCol_WindowBg]			= ImVec4(0.11f, 0.11f, 0.11f, 1.00f);
		colors[ImGuiCol_PopupBg]			= ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
		colors[ImGuiCol_Border]				= ImVec4(0.20f, 0.20f, 0.19f, 1.00f);
		colors[ImGuiCol_FrameBg]			= ImVec4(0.17f, 0.19f, 0.19f, 1.00f);
		colors[ImGuiCol_FrameBgHovered]		= ImVec4(0.23f, 0.27f, 0.28f, 1.00f);
		colors[ImGuiCol_FrameBgActive]		= ImVec4(0.23f, 0.33f, 0.36f, 1.00f);
		colors[ImGuiCol_TitleBg]			= ImVec4(0.10f, 0.12f, 0.12f, 1.00f);
		colors[ImGuiCol_TitleBgActive]		= ImVec4(0.25f, 0.13f, 0.11f, 1.00f);
		colors[ImGuiCol_MenuBarBg]			= ImVec4(0.19f, 0.21f, 0.22f, 1.00f);
		colors[ImGuiCol_ScrollbarBg]		= ImVec4(0.11f, 0.11f, 0.11f, 0.00f);
		colors[ImGuiCol_CheckMark]			= ImVec4(0.61f, 0.54f, 0.49f, 1.00f);
		colors[ImGuiCol_SliderGrab]			= ImVec4(0.61f, 0.54f, 0.49f, 1.00f);
		colors[ImGuiCol_SliderGrabActive]	= ImVec4(0.35f, 0.19f, 0.16f, 1.00f);
		colors[ImGuiCol_Button]				= ImVec4(0.18f, 0.21f, 0.25f, 1.00f);
		colors[ImGuiCol_ButtonHovered]		= ImVec4(0.31f, 0.38f, 0.47f, 1.00f);
		colors[ImGuiCol_ButtonActive]		= ImVec4(0.36f, 0.47f, 0.60f, 1.00f);
		colors[ImGuiCol_Header]				= ImVec4(0.18f, 0.21f, 0.25f, 1.00f);
		colors[ImGuiCol_HeaderHovered]		= ImVec4(0.31f, 0.38f, 0.47f, 1.00f);
		colors[ImGuiCol_HeaderActive]		= ImVec4(0.37f, 0.47f, 0.60f, 1.00f);
		colors[ImGuiCol_Separator]			= ImVec4(0.24f, 0.25f, 0.25f, 1.00f);
		colors[ImGuiCol_ResizeGrip]			= ImVec4(0.33f, 0.33f, 0.33f, 1.00f);
		colors[ImGuiCol_ResizeGripHovered]	= ImVec4(0.40f, 0.26f, 0.24f, 1.00f);
		colors[ImGuiCol_ResizeGripActive]	= ImVec4(0.42f, 0.13f, 0.10f, 1.00f);
		colors[ImGuiCol_Tab]				= ImVec4(0.37f, 0.34f, 0.32f, 1.00f);
		colors[ImGuiCol_TabHovered]			= ImVec4(0.51f, 0.29f, 0.20f, 1.00f);
		colors[ImGuiCol_TabActive]			= ImVec4(0.36f, 0.13f, 0.11f, 1.00f);
		colors[ImGuiCol_TabUnfocused]		= ImVec4(0.24f, 0.24f, 0.23f, 1.00f);
		colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.30f, 0.14f, 0.12f, 1.00f);
		colors[ImGuiCol_DockingPreview]		= ImVec4(0.44f, 0.31f, 0.27f, 0.70f);
		colors[ImGuiCol_DockingEmptyBg]		= ImVec4(0.08f, 0.08f, 0.08f, 1.00f);
		colors[ImGuiCol_NavHighlight]		= ImVec4(0.77f, 0.30f, 0.09f, 1.00f);
		colors[ImGuiCol_DragDropTarget]		= ImVec4(0.88f, 0.63f, 0.08f, 1.00f);
	}

	void UserInterface::StartFrame()
	{
		// Start the Dear ImGui frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

	}
	void UserInterface::EndFrame()
	{

		ImGuiIO& io = ImGui::GetIO();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		// Update and Render additional Platform Windows
		// (Platform functions may change the current OpenGL context, so we save/restore it to make it easier to paste this code elsewhere.
		//  For this specific demo app we could also call glfwMakeContextCurrent(window) directly)
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			GLFWwindow* backup_current_context = glfwGetCurrentContext();
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
			glfwMakeContextCurrent(backup_current_context);
		}
	}
} // !mnemosy::gui