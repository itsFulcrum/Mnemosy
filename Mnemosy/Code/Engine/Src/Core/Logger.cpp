#include "Engine/Include/Core/Logger.h"


#include "Engine/Include/Core/Log.h"
 
#include <spdlog/sinks/stdout_color_sinks.h>

#include <vector>
#include <memory>


namespace mnemosy::core
{
	Logger::Logger()
	{
		//auto consoleSink = std::make_shared
		auto consoleSink = std::make_shared<spdlog::sinks::stderr_color_sink_mt>();
		//std::shared_ptr<spdlog::sinks::stderr_color_sink_mt> consoleSink = std::make_shared<spdlog::sinks::stderr_color_sink_mt>();
		consoleSink->set_pattern("%^[%H:%M:%S.%e][%L] %v%$");
		
		std::vector<spdlog::sink_ptr> sinks{ consoleSink };

		auto logger = std::make_shared<spdlog::logger>(MNEMOSY_LOGGER_NAME, sinks.begin(),sinks.end());
		//std::shared_ptr<spdlog::logger> logger = std::make_shared<spdlog::logger>("MnemosyLogger", sinks.begin(),sinks.end());

		logger->set_level(spdlog::level::trace);
		logger->flush_on(spdlog::level::trace);
		spdlog::register_logger(logger);

	}

	Logger::~Logger()
	{
		spdlog::shutdown();
	}

}