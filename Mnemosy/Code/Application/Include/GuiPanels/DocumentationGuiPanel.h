#ifndef DOCUMENTATION_GUI_PANEL_H
#define DOCUMENTATION_GUI_PANEL_H

#include "Include/Gui/GuiPanel.h"

#include "string"

namespace mnemosy::gui
{

	class DocumentationGuiPanel : public GuiPanel {
	public:
		DocumentationGuiPanel();
		~DocumentationGuiPanel();

		virtual void Draw() override;

	private:

		void LoadTextFromFileToString();
		void LoadTextFileToString(const char* filepath, std::string& outString);

		std::string m_about_text;
		std::string m_getStarted_text;
		std::string m_controlls_text;
		std::string m_additionalInfo_text;
	};

} // !mnemosy::gui


#endif // !DOCUMENTATION_GUI_PANEL_H

