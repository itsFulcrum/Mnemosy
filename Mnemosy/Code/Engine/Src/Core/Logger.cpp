#include "Include/Core/Logger.h"

#include "Include/MnemosyConfig.h"
 
//#include <spdlog/sinks/stdout_color_sinks.h>
//#include <vector>



namespace mnemosy::core
{
	/*
	Logger::Logger() {
//		consoleSink = std::make_shared<spdlog::sinks::stderr_color_sink_mt>();
//		
//		
//		consoleSink->set_pattern("%^[%H:%M:%S.%e][%L] %v%$");
//		
//		sinks = std::vector<spdlog::sink_ptr>{ consoleSink };
//
//		logger = std::make_shared<spdlog::logger>(MNEMOSY_LOGGER_NAME, sinks.begin(),sinks.end());
//		
//		spdlog::get(MNEMOSY_LOGGER_NAME);
//
//#ifdef MNEMOSY_CONFIG_DEBUG
//		logger->set_level(spdlog::level::trace);
//		logger->flush_on(spdlog::level::trace);
//#endif // DEBUG
//
//#ifdef MNEMOSY_CONFIG_RELEASE
//		logger->set_level(spdlog::level::debug);
//		logger->flush_on(spdlog::level::debug);
//
//#endif // MNEMOSY_CONFIG_RELEASE
//
//
//
//
//		spdlog::register_logger(logger);

	}
	*/


	void LogCallback(flcrm::log::LogMessage msg) {

	}

	void Logger::Init() {


		flcrm::log::init();

		flcrm::log::LogDevice console_log_device;

		console_log_device.log_priority = flcrm::log::LogLevel::LEVEL_TRACE;
		console_log_device.output_type = flcrm::log::OutputType::CONSOLE_PRINT;
		console_log_device.print_to_console_in_color = true;

		console_log_device.format_info.add_time = true;
		console_log_device.format_info.add_source_file = false;
		console_log_device.format_info.add_source_function_name = true;
		console_log_device.format_info.add_source_line = false;
		console_log_device.format_info.add_log_level = true;
		console_log_device.format_info.time_format = "%H:%M:%S";

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

	//Logger::~Logger()
	//{

	//	//spdlog::drop_all();
	//	//
	//	//sinks.clear();
	//	//
	//	//spdlog::shutdown();
	//}

} // mnemosy::core