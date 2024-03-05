#ifndef LOGGER_H
#define LOGGER_H



#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/async.h>
#include <memory>
#include <vector>

namespace mnemosy::core
{
	class Logger
	{
	public:
		Logger();
		~Logger();

	private:
		std::shared_ptr<spdlog::sinks::stderr_color_sink_mt> consoleSink;
		std::shared_ptr<spdlog::logger> logger;
		std::vector<spdlog::sink_ptr> sinks;
	};
} // menmosy::core

#endif // !LOGGER_H
