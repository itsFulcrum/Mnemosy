#include "Include/Core/Logger.h"

#include "Include/Core/Log.h"
 
//#include <spdlog/sinks/stdout_color_sinks.h>
#include <vector>



namespace mnemosy::core
{
	Logger::Logger() {
		consoleSink = std::make_shared<spdlog::sinks::stderr_color_sink_mt>();
		
		
		consoleSink->set_pattern("%^[%H:%M:%S.%e][%L] %v%$");
		
		sinks = std::vector<spdlog::sink_ptr>{ consoleSink };

		logger = std::make_shared<spdlog::logger>(MNEMOSY_LOGGER_NAME, sinks.begin(),sinks.end());
		
		logger->set_level(spdlog::level::trace);
		logger->flush_on(spdlog::level::trace);
		spdlog::register_logger(logger);

	}

	Logger::~Logger()
	{
		spdlog::drop_all();

		sinks.clear();

		spdlog::shutdown();
	}

} // mnemosy::core