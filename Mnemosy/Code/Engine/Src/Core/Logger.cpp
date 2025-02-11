#include "Include/Core/Logger.h"

#include "Include/MnemosyConfig.h"

#include <filesystem>

namespace mnemosy::core
{
	
	void LogCallback(flcrm::log::LogMessage msg) {

	}

	void Logger::Init() {



		flcrm::log::init();

		flcrm::log::LogDevice console_log_device;

#ifdef MNEMOSY_CONFIG_RELEASE
		console_log_device.log_priority = flcrm::log::LogLevel::LEVEL_DEBUG;
		console_log_device.output_type = flcrm::log::OutputType::CONSOLE_PRINT;
		console_log_device.print_to_console_in_color = true;

		console_log_device.format_info.add_time = true;
		console_log_device.format_info.add_source_file = false;
		console_log_device.format_info.add_source_function_name = false;
		console_log_device.format_info.add_source_line = false;
		console_log_device.format_info.add_log_level = true;
		console_log_device.format_info.time_format = "%H:%M:%S";
#else //MNEMOSY_CONFIG_DEBUG
		console_log_device.log_priority = flcrm::log::LogLevel::LEVEL_TRACE;
		console_log_device.output_type = flcrm::log::OutputType::CONSOLE_PRINT;
		console_log_device.print_to_console_in_color = true;

		console_log_device.format_info.add_time = true;
		console_log_device.format_info.add_source_file = false;
		console_log_device.format_info.add_source_function_name = true;
		console_log_device.format_info.add_source_line = false;
		console_log_device.format_info.add_log_level = true;
		console_log_device.format_info.time_format = "%H:%M:%S";
#endif

		m_console_log_device_id = flcrm::log::device_register(console_log_device);

		// ============================   File logger
		{

			namespace fs = std::filesystem;

			fs::path logsFolder = fs::path("../Resources/Logs");

			bool logsFolderExists = true;

			if (!fs::exists(logsFolder)) {
				
				try {
					fs::create_directories(logsFolder);
					logsFolderExists = true;
				}
				catch (fs::filesystem_error err) {
					flcrm_log_fatal("LOG: Failed to create logs folder!");
					logsFolderExists = false;
				}
			}


			m_log_file_path = logsFolder / fs::path("Mnemosy_Session_Log.txt");

			if (logsFolderExists) {
				m_out_log_file.open(m_log_file_path);
			}			

			flcrm::log::LogDevice file_log_device;

			file_log_device.log_priority = flcrm::log::LogLevel::LEVEL_TRACE;
			file_log_device.output_type = flcrm::log::OutputType::FUNCTION_CALLBACK;
			file_log_device.format_info.add_time = true;
			file_log_device.format_info.add_source_file = false;
			file_log_device.format_info.add_source_function_name = true;
			file_log_device.format_info.add_source_line = false;
			file_log_device.format_info.add_log_level = true;
			file_log_device.format_info.time_format = "%Y-%m-%d %H:%M:%S";

			file_log_device.function_callback = std::bind(&mnemosy::core::Logger::Log_File_Callback, this, std::placeholders::_1);
			
			m_file_log_device_id = flcrm::log::device_register(file_log_device);
		}




		// ============================   GUI logger
		{
			flcrm::log::LogDevice gui_log_device;

			gui_log_device.log_priority = flcrm::log::LogLevel::LEVEL_INFO;
			gui_log_device.output_type = flcrm::log::OutputType::FUNCTION_CALLBACK;
			gui_log_device.format_info.add_time = true;
			gui_log_device.format_info.add_source_file = false;
			gui_log_device.format_info.add_source_function_name = false;
			gui_log_device.format_info.add_source_line = false;
			gui_log_device.format_info.add_log_level = true;
			gui_log_device.format_info.time_format = "%H:%M:%S";

			gui_log_device.function_callback = std::bind(&mnemosy::core::Logger::Log_Gui_Callback, this, std::placeholders::_1);

			m_gui_log_device_id = flcrm::log::device_register(gui_log_device);
		}


		// ============================   POPUP logger
		{
			flcrm::log::LogDevice popup_log_device;

			popup_log_device.log_priority = flcrm::log::LogLevel::LEVEL_CUSTOM;
			popup_log_device.log_maxLevel = flcrm::log::LogLevel::LEVEL_CUSTOM;
			popup_log_device.output_type = flcrm::log::OutputType::FUNCTION_CALLBACK;
			popup_log_device.format_info.add_time = false;
			popup_log_device.format_info.add_source_file = false;
			popup_log_device.format_info.add_source_function_name = false;
			popup_log_device.format_info.add_source_line = false;
			popup_log_device.format_info.add_log_level = true;
			//popup_log_device.format_info.time_format = "";

			popup_log_device.function_callback = std::bind(&mnemosy::core::Logger::Log_Popup_Callback, this, std::placeholders::_1);

			m_gui_log_device_id = flcrm::log::device_register(popup_log_device);
		}




	}


	void Logger::Shutdown() {

		flcrm::log::shutdown();

		m_console_log_device_id = 0;
		m_file_log_device_id = 0;

		m_gui_log_messages.clear();

		if (m_out_log_file.is_open()) {
			m_out_log_file.close();
		}

	}

	void Logger::Log_File_Callback(flcrm::log::LogMessage& msg) {

		if (m_out_log_file.is_open()) {
			
			m_out_log_file << msg.formated_message << '\n';

			if (msg.log_level >= flcrm::log::LogLevel::LEVEL_WARN) {
				m_out_log_file.flush();
			}
		}
	}

	void Logger::Log_Gui_Callback(flcrm::log::LogMessage& msg) {

		m_gui_log_messages.push_back(msg);
	}

	void Logger::Log_Popup_Callback(flcrm::log::LogMessage& msg) {

		m_popup_message_triggered = true;
		m_popupMessage = msg.formated_message;
	}

} // mnemosy::core