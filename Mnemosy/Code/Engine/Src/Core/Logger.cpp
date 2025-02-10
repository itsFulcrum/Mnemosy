#include "Include/Core/Logger.h"

#include "Include/MnemosyConfig.h"

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

		// user log device
		flcrm::log::LogDevice user_log_device;

		user_log_device.log_priority = flcrm::log::LogLevel::LEVEL_TRACE;
		user_log_device.output_type = flcrm::log::OutputType::FUNCTION_CALLBACK;
		user_log_device.format_info.add_time = true;
		user_log_device.format_info.add_source_file = false;
		user_log_device.format_info.add_source_function_name = false;
		user_log_device.format_info.add_source_line = false;
		user_log_device.format_info.add_log_level = true;
		user_log_device.format_info.time_format = "%Y-%m-%d %H:%M:%S";

		user_log_device.function_callback = std::bind(&mnemosy::core::Logger::LogCallback, this, std::placeholders::_1);
		
		m_file_log_device_id = flcrm::log::device_register(user_log_device);
	}

	void Logger::Shutdown()
	{
		flcrm::log::shutdown();

		m_console_log_device_id = 0;
		m_file_log_device_id = 0;
	}

	void Logger::LogCallback(flcrm::log::LogMessage& msg)
	{

		//printf("Logging the message from callback: %s \n", msg.formated_message.c_str());
	}

} // mnemosy::core