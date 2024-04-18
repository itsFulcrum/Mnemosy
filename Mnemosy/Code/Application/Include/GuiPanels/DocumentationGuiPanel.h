#ifndef DOCUMENTATION_GUI_PANEL_H
#define DOCUMENTATION_GUI_PANEL_H

#include "Include/Gui/GuiPanel.h"

#include "string"

namespace mnemosy::gui
{
	class DocumentationGuiPanel : public GuiPanel
	{
	public:
		DocumentationGuiPanel();
		~DocumentationGuiPanel();


		virtual void Draw() override;

	private:
		void LoadTextFromFileToString();
		void LoadTextFileToString(const char* filepath, std::string& outString);

		std::string m_AboutMnemosy_text;
		std::string m_GettingStarted_text;
		std::string m_KeyboardControlls_text;
		std::string m_AdditionalInformation_text;
	};

} // !mnemosy::gui


#endif // !DOCUMENTATION_GUI_PANEL_H

