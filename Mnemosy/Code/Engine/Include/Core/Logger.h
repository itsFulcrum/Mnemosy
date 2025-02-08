#ifndef LOGGER_H
#define LOGGER_H

#include "Include/Core/Log.h"


//#include <spdlog/sinks/stdout_color_sinks.h>
//#include <spdlog/async.h>
//#include <memory>
//#include <vector>

namespace mnemosy::core
{
	class Logger
	{
	public:
		Logger() = default;
		~Logger() = default;

		void Init();

		void Shutdown();

		void LogCallback(flcrm::log::LogMessage& msg);

	private:

		unsigned int m_console_log_device_id = 0;
		unsigned int m_file_log_device_id = 0;

		//std::shared_ptr<spdlog::sinks::stderr_color_sink_mt> consoleSink;
		//std::shared_ptr<spdlog::logger> logger;
		//std::vector<spdlog::sink_ptr> sinks;
	};
} // menmosy::core

#endif // !LOGGER_H
