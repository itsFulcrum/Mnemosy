#ifndef FLCRM_UTILS_LOG
#define FLCRM_UTILS_LOG

#include <string>
#include <format>



namespace flcrm::log {

	enum flcrm_log_level {
		FLCRM_LOG_LEVEL_TRACE	= 0,
		FLCRM_LOG_LEVEL_DEBUG	= 1,
		FLCRM_LOG_LEVEL_INFO	= 2,
		FLCRM_LOG_LEVEL_WARN	= 3,
		FLCRM_LOG_LEVEL_ERROR	= 4,
		FLCRM_LOG_LEVEL_FATAL	= 5
	};

	// we want some was to let users specify how they want to output
	// like to console or to file or whatever
	// this struct should then contain information of what log level to output, it needs a format info, and where to output (maybe a function pointer?)
	//struct LogOutputter {

	//};


	struct LogFormatInfo {
		bool add_time = false;
		bool add_year = false;
		bool add_month = false;
		bool add_day = false;
		
		bool add_hour = false;
		bool add_minute = false;
		bool add_second = false;
		bool add_milisecond = false;

		bool add_log_level = false;
		bool add_source_file = false;
		bool add_source_function_name = false;
		bool add_source_line = false;
	};


	static void console_printer(flcrm_log_level logLevel, bool print_in_color, std::string& msg) {

		if (!print_in_color) {
			printf("%s", msg.c_str());

		}
		else {
			switch (logLevel)
			{
			case flcrm::log::flcrm_log_level::FLCRM_LOG_LEVEL_TRACE:
				printf("%s\n", msg.c_str());
				break;
			case flcrm::log::flcrm_log_level::FLCRM_LOG_LEVEL_DEBUG:
				printf("\x1B[36m%s\033[0m\n", msg.c_str());
				break;
			case flcrm::log::flcrm_log_level::FLCRM_LOG_LEVEL_INFO:
				printf("\x1B[32m%s\033[0m\n", msg.c_str());
				break;
			case flcrm::log::flcrm_log_level::FLCRM_LOG_LEVEL_WARN:
				printf("\x1B[33m%s\033[0m\n", msg.c_str());
				break;
			case flcrm::log::flcrm_log_level::FLCRM_LOG_LEVEL_ERROR:
				printf("\x1B[31m%s\033[0m\n", msg.c_str());
				break;
			case flcrm::log::flcrm_log_level::FLCRM_LOG_LEVEL_FATAL:
				printf("\x1B[3;41;37m%s\033[0m\n", msg.c_str());
				break;
			default:
				break;
			}
		}

		// colored printf
		//printf("\x1B[36m[DEBUG] \033[0m\n");
		//printf("\x1B[32m[INFO] \033[0m\n");
		//printf("\x1B[33m[WARN] \033[0m\n");
		//printf("\x1B[31m[ERROR] \033[0m\n");
		//printf("\x1B[3;41;37m[FATAL]\033[0m\n");

	}





	template<typename... Args>
	static void log_print(flcrm_log_level log_level,bool print_in_color,const char* source_file, const char* source_function_name, int source_line, LogFormatInfo& format_info,std::string_view format_string, Args&& ... args)
	{
		
		//std::string logMsg = std::vformat(format_string, std::make_format_args(args...));



		//std::string out = "[DATE:TIME][TRACE] File: " + std::string(file) + " Line: " + std::to_string(line) + " msg: " + logMsg;






		std::string base;
		
		if (format_info.add_time) {
			base += "[TIME] ";
		}
		//std::string base = "[yyyy::mm::dd::hh::mm::ss::ms] " + logMsg;

		if (format_info.add_log_level) {

			switch (log_level)
			{
			case flcrm::log::FLCRM_LOG_LEVEL_TRACE:
				base += "[TRACE]";
				break;
			case flcrm::log::FLCRM_LOG_LEVEL_DEBUG:
				base += "[DEBUG]";
				break;
			case flcrm::log::FLCRM_LOG_LEVEL_INFO:
				base += "[INFO]";
				break;
			case flcrm::log::FLCRM_LOG_LEVEL_WARN:
				base += "[WARN]";
				break;
			case flcrm::log::FLCRM_LOG_LEVEL_ERROR:
				base += "[ERROR]";
				break;
			case flcrm::log::FLCRM_LOG_LEVEL_FATAL:
				base += "[FATAL]";
				break;
			default:
				break;
			}
		}


		if (format_info.add_source_file) {
			base += "[File: " + std::string(source_file) + "]";
		}


		if (format_info.add_source_function_name) {
			base += "[Func: " + std::string(source_function_name) + "]";
		}

		if (format_info.add_source_line) {
			base += "[Line: " + std::to_string(source_line) + "]";
		}
		

		base += " " + std::vformat(format_string, std::make_format_args(args...));



		console_printer(log_level, print_in_color ,base);
	}




	// example log macros
//#define flcrm_log_trace(...) log_print(flcrm_log_level::FLCRM_LOG_LEVEL_TRACE,true,__FILE__,,__LINE__,__VA_ARGS__)
//#define flcrm_log_debug(...) log_print(flcrm_log_level::FLCRM_LOG_LEVEL_DEBUG,true,__FILE__,,__LINE__,__VA_ARGS__)
//#define flcrm_log_info(...)  log_print(flcrm_log_level::FLCRM_LOG_LEVEL_INFO ,true,__FILE__,,__LINE__,__VA_ARGS__)
//#define flcrm_log_warn(...)  log_print(flcrm_log_level::FLCRM_LOG_LEVEL_WARN ,true,__FILE__,,__LINE__,__VA_ARGS__)
//#define flcrm_log_error(...) log_print(flcrm_log_level::FLCRM_LOG_LEVEL_ERROR,true,__FILE__,,__LINE__,__VA_ARGS__)
//#define flcrm_log_fatal(...) log_print(flcrm_log_level::FLCRM_LOG_LEVEL_FATAL,true,__FILE__,,__LINE__,__VA_ARGS__)

	static void log_test() {

		std::string a = "Int: {}, float {}, double: {}, std::string: {}, c_str: {}";

		const char* c = "ceeeString";

		int b = 10;
		float f = 0.1f;
		double d = 2.00000000001;

		std::string s = "hellope str";


		LogFormatInfo info;
		//flcrm_log_trace(info,a, b, f, d, s, c);
	}



	//class logger
	//{
	//public:
	//	logger() = default;
	//	~logger() = default;

	//private:

	//};
}


#endif // !FLCRM_UTILS_LOG
