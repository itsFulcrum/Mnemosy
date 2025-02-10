#ifndef LOGGER_H
#define LOGGER_H

#include "Include/Core/Log.h"

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
	};
} // menmosy::core

#endif // !LOGGER_H
