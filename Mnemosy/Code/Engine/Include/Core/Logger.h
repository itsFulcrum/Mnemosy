#ifndef LOGGER_H
#define LOGGER_H

#include "Include/Core/Log.h"
#include <vector>
#include <fstream>
#include <string>
#include <filesystem>

namespace mnemosy::core
{
	class Logger
	{
	public:
		Logger() = default;
		~Logger() = default;

		void Init();

		void Shutdown();


		std::vector<flcrm::log::LogMessage>& Get_GuiLogMessages() { return m_gui_log_messages; }


		bool IsPopupMessageTriggered() { return m_popup_message_triggered; }
		void UntriggerPopupMessage() { m_popup_message_triggered = false; }
		std::string& GetPopupMessage() { return m_popupMessage; }


	private:

		void Log_File_Callback(flcrm::log::LogMessage& msg);

		void Log_Gui_Callback(flcrm::log::LogMessage& msg);
		
		void Log_Popup_Callback(flcrm::log::LogMessage& msg);

	private:
		std::vector<flcrm::log::LogMessage> m_gui_log_messages;

		std::ofstream m_out_log_file;
		std::filesystem::path m_log_file_path;

		unsigned int m_console_log_device_id = 0;
		unsigned int m_gui_log_device_id = 0;
		unsigned int m_file_log_device_id = 0;
		unsigned int m_popup_log_device_id = 0;

		bool m_popup_message_triggered = false;
		std::string m_popupMessage;
	};
} // menmosy::core

#endif // !LOGGER_H
