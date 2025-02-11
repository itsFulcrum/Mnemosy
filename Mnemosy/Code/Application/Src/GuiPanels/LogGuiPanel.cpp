#include "Include/GuiPanels/LogGuiPanel.h"
#include "Include/GuiPanels/GuiPanelsCommon.h"

#include "Include/MnemosyConfig.h"
#include "Include/MnemosyEngine.h"
#include "Include/Core/Logger.h"

#include "Include/Core/Log.h"

#include "ImGui/imgui.h"
#include "ImGui/imgui_stdlib.h"

#include <FulcrumUtils/Flcrm_Log.hpp>
#include <vector>




namespace mnemosy::gui {

	LogGuiPanel::LogGuiPanel() {
		panelName = "Log";
		panelType = MNSY_GUI_PANEL_LOG;

		//MNEMOSY_TRACE(	"Gui Log Test Trace	");
		//MNEMOSY_DEBUG(	"Gui Log Test Debug	");
		//MNEMOSY_INFO(		"Gui Log Test Info	");
		//MNEMOSY_WARN(		"Gui Log Test Warn	");
		//MNEMOSY_ERROR(	"Gui Log Test Error	");
		//MNEMOSY_CRITICAL(	"Gui Log Test Fatal	");
	}

	LogGuiPanel::~LogGuiPanel() {

	}

	void LogGuiPanel::Draw() {

		if (!showPanel)
			return;

		// If performance becomes an issure we can think about using ImGuiTextBuffer instead with clipping, 
		// I belive it'll then only submit draw calls for the actually visible message. 

		const static ImVec4 colors[6] = { Gui_Txt_Color_Trace ,Gui_Txt_Color_Debug ,Gui_Txt_Color_Info ,Gui_Txt_Color_Warn ,Gui_Txt_Color_Error,Gui_Txt_Color_Fatal };

		ImGui::Begin(panelName,&showPanel);
		{
			ImGui::SeparatorText("Log Messages");
			ImGui::Spacing();


			std::vector<flcrm::log::LogMessage>& logs = MnemosyEngine::GetInstance().GetLogger().Get_GuiLogMessages();

			for (unsigned int i = 0; i < logs.size(); i++) {

				ImVec4 col = colors[(unsigned int)logs[i].log_level];

				ImGui::PushStyleColor(ImGuiCol_Text, col);

				ImGui::TextWrapped(logs[i].formated_message.c_str());

				ImGui::PopStyleColor();
			}

		} 
		ImGui::End();

	}


}