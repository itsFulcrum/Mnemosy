#ifndef FLCRM_UTILS_LOG
#define FLCRM_UTILS_LOG

#pragma once

#include <string>
#include <format>
#include <chrono>
#include <ctime>
#include <vector>
#include <functional>
#include <mutex>

#include <assert.h>


// ============================================================================
// ============================================================================
// Fulcrum simple logging single header
// 
// how to use example at the bottom of the file
// TLDR:
// - use predefined macros to log directly to console with default formatting
// - or create and configure a LogDevice then register it with flcrm::log::device_register(LogDevice);
// - then use flcrm::log::log(..) to log messages to all registered devices using std::format string formatting
// - create your own macros or wrappers around flcrm::log::log();



// define this macro to disable default console logging capabilites
//#define FLCRM_LOG_DISABLE_DEFAULT_CONSOLE_LOGGING

// you may define any of these macros before this include to configure the default logging formatting

// #define flcrm_default_log_format_timeFormat				"%Y-%m-%d %H:%M:%S"		// timestamp format using std::strftime()
// #define flcrm_default_log_format_logTimestap				true					// if to add timestamp
// #define flcrm_default_log_format_logLevel				true					// if to add log level
// #define flcrm_default_log_format_logFile					false					// if to add filepath
// #define flcrm_default_log_format_logLine					false					// if to add source line
// #define flcrm_default_log_format_logFunctionName			false					// if to add source function name
// #define flcrm_default_log_format_printToConsoleInColor	true					// print to console in color if supported

// ============================================================================
// ============================================================================



namespace flcrm::log {

	enum LogLevel {
		LEVEL_TRACE	= 0,
		LEVEL_DEBUG	= 1,
		LEVEL_INFO	= 2,
		LEVEL_WARN	= 3,
		LEVEL_ERROR	= 4,
		LEVEL_FATAL	= 5
	};

	enum OutputType {
		CONSOLE_PRINT		= 0,
		FUNCTION_CALLBACK	= 1,
		//FILE_PRINT		= 2
	};

	struct LogFormatInfo {
	public:
		std::string time_format = "%Y-%m-%d %H:%M:%S"; // default converts like this 'yyyy-mm-dd hh:mm:ss' -> '2025-12-31 23:21:25' | uses std::strftime() function
		bool add_time = false;

		bool add_log_level = false;
		bool add_source_file = false;
		bool add_source_function_name = false;
		bool add_source_line = false;
	};
	
	struct LogMessage {
	public:
		LogLevel log_level;
		std::string formated_message;
	};

	struct LogDevice {
	public:
		LogFormatInfo format_info;
		LogLevel log_priority = LogLevel::LEVEL_TRACE;
		OutputType output_type = OutputType::CONSOLE_PRINT;
		bool print_to_console_in_color = false;
		std::function<void(LogMessage&)> function_callback = nullptr;
	};

	namespace flcrm_internal {

		class DeviceRegister {
		private:
			static inline std::vector<LogDevice> m_registered_devices;
			static inline std::mutex m_log_mutex;

		public:

			static void init() {

			}

			static void shutdown() {

				m_registered_devices.clear();
			}

			static unsigned int device_register(LogDevice& device) {

				m_registered_devices.push_back(device);

				return m_registered_devices.size() -1;
			}

			static LogDevice& device_get(unsigned int id) {
			
				// bounds checking
				if (id >= m_registered_devices.size() || id < 0) {
				
					assert("logger id is out of bounds, no device is registered with this id");
				}

				return m_registered_devices[id];
			}

			static unsigned int num_devices_get() {
				return m_registered_devices.size();
			}


			static std::mutex& log_mutex_get() {
				return m_log_mutex;
			}

		};
	}

	static void init() {
		flcrm_internal::DeviceRegister::init();
	}

	static unsigned int device_register(LogDevice& device) {
		return flcrm_internal::DeviceRegister::device_register(device);
	}

	static LogDevice& device_get(unsigned int id) {
		return flcrm_internal::DeviceRegister::device_get(id);
	}

	static void shutdown() {
		flcrm_internal::DeviceRegister::shutdown();
	}


	static void console_printer(LogMessage& log_message, bool print_in_color)
	{

		if (!print_in_color) {
			printf("%s", log_message.formated_message.c_str());

		}
		else {
			switch (log_message.log_level)
			{
			case flcrm::log::LogLevel::LEVEL_TRACE:
				printf("%s\n", log_message.formated_message.c_str());
				break;
			case flcrm::log::LogLevel::LEVEL_DEBUG:
				printf("\x1B[36m%s\033[0m\n", log_message.formated_message.c_str());
				break;
			case flcrm::log::LogLevel::LEVEL_INFO:
				printf("\x1B[32m%s\033[0m\n", log_message.formated_message.c_str());
				break;
			case flcrm::log::LogLevel::LEVEL_WARN:
				printf("\x1B[33m%s\033[0m\n", log_message.formated_message.c_str());
				break;
			case flcrm::log::LogLevel::LEVEL_ERROR:
				printf("\x1B[31m%s\033[0m\n", log_message.formated_message.c_str());
				break;
			case flcrm::log::LogLevel::LEVEL_FATAL:
				printf("\x1B[3;41;37m%s\033[0m\n", log_message.formated_message.c_str());
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


	static std::string log_format_info_string(LogLevel log_level, LogFormatInfo& format_info, const char* source_file, const char* source_function_name, int source_line)
	{

		uint32_t reserve_amount = 16;

		if (format_info.add_time) {
			reserve_amount += 64 + 2;
		}

		if (format_info.add_log_level) {
			reserve_amount += 7;
		}

		if (format_info.add_source_line) {
			reserve_amount += 8 + 4;
		}

		if (format_info.add_source_file) {
			reserve_amount += 64;
		}

		if (format_info.add_source_function_name) {
			reserve_amount += 64;
		}

		std::string base;
		base.reserve(reserve_amount);

		if (format_info.add_time) {

			std::chrono::system_clock::time_point time_point = std::chrono::system_clock::now();

			std::time_t now = std::chrono::system_clock::to_time_t(time_point);

			//uint64_t millis = std::chrono::duration_cast<std::chrono::milliseconds>(time_point.time_since_epoch()).count() % 1000;

			const unsigned int max_time_length = 64;
			char buf[max_time_length];

			unsigned int i = std::strftime(&buf[0], max_time_length, format_info.time_format.c_str(), std::localtime(&now));
			if (i == 0)  {
				//"Max Time Length Is 64 chars";

				buf[0] = 'M'; 
				buf[1] = 'a';
				buf[2] = 'x';
				buf[3] = ' ';
				buf[4] = 'T';
				buf[5] = 'i';
				buf[6] = 'm';
				buf[7] = 'e';
				buf[8] = ' ';
				buf[9] = 'L';
				buf[10] = 'e';
				buf[11] = 'n';
				buf[12] = 'g';
				buf[13] = 't';
				buf[14] = 'h';
				buf[15] = ' ';
				buf[16] = 'i';
				buf[17] = 's';
				buf[18] = ' ';
				buf[19] = '6';
				buf[20] = '4';
				buf[21] = ' ';
				buf[22] = 'c';
				buf[23] = 'h';
				buf[24] = 'a';
				buf[25] = 'r';
				buf[26] = 's';
				buf[27] = '\0';
			}

			base += "[" + std::string(buf) + "]";
		}
		
		if (format_info.add_log_level) {

			switch (log_level)
			{
			case flcrm::log::LEVEL_TRACE:
				base += "[TRACE]";
				break;
			case flcrm::log::LEVEL_DEBUG:
				base += "[DEBUG]";
				break;
			case flcrm::log::LEVEL_INFO:
				base += "[INFO]";
				break;
			case flcrm::log::LEVEL_WARN:
				base += "[WARN]";
				break;
			case flcrm::log::LEVEL_ERROR:
				base += "[ERROR]";
				break;
			case flcrm::log::LEVEL_FATAL:
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


		return base;
	}


	static void log_device(LogDevice& device, LogMessage& message) {

		// do mutex here 
		std::scoped_lock lock(flcrm_internal::DeviceRegister::log_mutex_get());


		if (device.output_type == OutputType::CONSOLE_PRINT) {

			console_printer(message, device.print_to_console_in_color);

		}
		else if (device.output_type == OutputType::FUNCTION_CALLBACK) {
			if (device.function_callback) {
				device.function_callback(message);
			}
		}
	}



	// log to all registered devices
	template<typename... Args>
	static void log(LogLevel log_level, const char* source_file, int source_line, const char* source_function_name, std::string_view format_string, Args&& ... args)
	{

		unsigned int num_devices = flcrm_internal::DeviceRegister::num_devices_get();

		for (int deviceId = 0; deviceId < num_devices; deviceId++) {

			LogDevice& device = flcrm_internal::DeviceRegister::device_get(deviceId);

			if (log_level >= device.log_priority) {
				
				LogMessage msg;

				msg.log_level = log_level;
				msg.formated_message = log_format_info_string(log_level, device.format_info, source_file, source_function_name, source_line) + " " + std::vformat(format_string, std::make_format_args(args...));

				log_device(device, msg);
			}
		}
	}
	

	// log to consol directly 
	template<typename... Args>
	static void log_console_direct(LogLevel log_level,bool print_in_color,const char* source_file, int source_line, const char* source_function_name, LogFormatInfo& format_info,std::string_view format_string, Args&& ... args)
	{		
		LogMessage msg;
		msg.log_level = log_level;
		msg.formated_message = log_format_info_string(log_level,format_info, source_file, source_function_name, source_line) + " " + std::vformat(format_string, std::make_format_args(args...));


		std::scoped_lock lock(flcrm_internal::DeviceRegister::log_mutex_get());
		console_printer(msg, print_in_color);
	}




#ifndef FLCRM_LOG_DISABLE_DEFAULT_CONSOLE_LOGGING

#ifndef flcrm_default_log_format_timeFormat
#define flcrm_default_log_format_timeFormat "%Y-%m-%d %H:%M:%S"
#endif // !flcrm_default_log_format_timeFormat

#ifndef flcrm_default_log_format_logTimestap
	#define flcrm_default_log_format_logTimestap true
#endif // !flcrm_default_log_format_logTimestap

#ifndef flcrm_default_log_format_logLevel
	#define flcrm_default_log_format_logLevel true
#endif // !flcrm_default_log_format_logLevel

#ifndef flcrm_default_log_format_logFile
	#define flcrm_default_log_format_logFile false
#endif // !flcrm_default_log_format_logFile

#ifndef flcrm_default_log_format_logLine
#define flcrm_default_log_format_logLine false
#endif // !flcrm_default_log_format_logLine

#ifndef flcrm_default_log_format_logFunctionName
	#define flcrm_default_log_format_logFunctionName false
#endif // !flcrm_default_log_format_logFunctionName

#ifndef flcrm_default_log_format_printToConsoleInColor
	#define flcrm_default_log_format_printToConsoleInColor true
#endif // !flcrm_default_log_format_printToConsoleInColor


// default log message format info

static inline LogFormatInfo flcrm_default_format_info{ flcrm_default_log_format_timeFormat ,flcrm_default_log_format_logTimestap,flcrm_default_log_format_logLevel,flcrm_default_log_format_logFile, flcrm_default_log_format_logFunctionName, flcrm_default_log_format_logLine };


	#ifdef WIN32   //WINDOWS
		#define FLCRM_LOG_FUNCTION_NAME   __FUNCTION__  
		#define flcrm_log_break __debugbreak(); 


	#else          //Linux
		#define FLCRM_LOG_FUNCTION_NAME   __func__
		#define flcrm_log_break __builtin_trap(); 
	#endif


		// macros for logging directly to consol
	#define flcrm_log_trace(...)	flcrm::log::log_console_direct(flcrm::log::LogLevel::LEVEL_TRACE,flcrm_default_log_format_printToConsoleInColor,__FILE__,__LINE__,FLCRM_LOG_FUNCTION_NAME,flcrm::log::flcrm_default_format_info,__VA_ARGS__)
	#define flcrm_log_debug(...)	flcrm::log::log_console_direct(flcrm::log::LogLevel::LEVEL_DEBUG,flcrm_default_log_format_printToConsoleInColor,__FILE__,__LINE__,FLCRM_LOG_FUNCTION_NAME,flcrm::log::flcrm_default_format_info,__VA_ARGS__)
	#define flcrm_log_info(...)		flcrm::log::log_console_direct(flcrm::log::LogLevel::LEVEL_INFO ,flcrm_default_log_format_printToConsoleInColor,__FILE__,__LINE__,FLCRM_LOG_FUNCTION_NAME,flcrm::log::flcrm_default_format_info,__VA_ARGS__)
	#define flcrm_log_warn(...)		flcrm::log::log_console_direct(flcrm::log::LogLevel::LEVEL_WARN ,flcrm_default_log_format_printToConsoleInColor,__FILE__,__LINE__,FLCRM_LOG_FUNCTION_NAME,flcrm::log::flcrm_default_format_info,__VA_ARGS__)
	#define flcrm_log_error(...)	flcrm::log::log_console_direct(flcrm::log::LogLevel::LEVEL_ERROR,flcrm_default_log_format_printToConsoleInColor,__FILE__,__LINE__,FLCRM_LOG_FUNCTION_NAME,flcrm::log::flcrm_default_format_info,__VA_ARGS__)
	#define flcrm_log_fatal(...)	flcrm::log::log_console_direct(flcrm::log::LogLevel::LEVEL_FATAL,flcrm_default_log_format_printToConsoleInColor,__FILE__,__LINE__,FLCRM_LOG_FUNCTION_NAME,flcrm::log::flcrm_default_format_info,__VA_ARGS__)

	#define flcrm_log_assert(x,msg) if((!x)) { flcrm_log_fatal("ASSERT - {}\n\t{}\n\tin file {}\n\ton line {}", #x, msg, __FILE__, __LINE__,FLCRM_LOG_FUNCTION_NAME); flcrm_log_break}

#endif // !FLCRM_LOG_DISABLE_DEFAULT_CONSOLE_LOGGING

}


#endif // !FLCRM_UTILS_LOG




// ====================================================
// 
//				USAGE EXAMPLE
// 
// ====================================================


/*

void console_example() {

	// ======== Using default console logging =========

	// if FLCRM_LOG_DISABLE_DEFAULT_CONSOLE_LOGGING is undefined you can use the following macros

	flcrm_log_trace("hellope");
	flcrm_log_debug("integer {}", 10);
	flcrm_log_info("string {}", std::string("hellope"));
	flcrm_log_warn("c-string {}, float {}", "c-string", 1.55f);
	flcrm_log_error("Eeeeerrrorr msg: {}", "error message");
	flcrm_log_fatal("not gud!");

	bool thisShouldBeTrue = true;
	flcrm_log_assert(thisShouldBeTrue,"Oh no its not true!"); // assert will break the program in the debugger and also print to console using flcrm_log_fatal;


	// ======== SETTING UP LOG DEVICES =========
	
	flcrm::log::init(); // this call does nothing but it feels good to initialize stuff.

	// create a new device
	flcrm::log::LogDevice myDevice;

	myDevice.output_type = flcrm::log::CONSOLE_PRINT; // set this device to print to console
	myDevice.log_priority = flcrm::log::LogLevel::LEVEL_INFO; // set the log priority -> logs with lower priority will not be outputtet.


	myDevice.print_to_console_in_color = true; // print in color if console supports it.

	// setup formatting
	myDevice.format_info.add_time = true; // add timestamp at the begginning.
	// specifiy the timestamp format. this format string is used with the std::strftime() function.
	myDevice.format_info.time_format = "%Y-%m-%d %H:%M:%S";  // this default ("%Y-%m-%d %H:%M:%S") converts like this '[yyyy-mm-dd hh:mm:ss]' -> '[2025-12-31 23:21:25]'

	myDevice.format_info.add_log_level = true;	 // adds log level stamp e.g: "[Trace] ..rest of the message";
	myDevice.format_info.add_source_file = true; // adds the source file if provided with e.g: __FILE__
	myDevice.format_info.add_source_line = true; // adds the source line if provided with e.g: __LINE__
	myDevice.format_info.add_source_function_name = true; // adds source function name if provided.   on windows may use: __FUNCTION__

	// register the device with the logger;
	unsigned int myDevice_id = flcrm::log::device_register(myDevice);

	// myDevice can now be deleted or go out of scope.

	// use id to get access and change the device settings.
	flcrm::log::LogDevice& referanceToLogDevice = flcrm::log::device_get(myDevice_id);
	referanceToLogDevice.print_to_console_in_color = false; // actually i dont want color anymore..
	

	// LOGGING

	std::string logMessage = "this is the message i want to log.";
	
	// use this function to log something to all registered devices
	flcrm::log::log(flcrm::log::LogLevel::LEVEL_INFO,__FILE__,__LINE__,__FUNCTION__,logMessage);

	// formatting uses std::format
	// you may add as many arguments as you want.

	int wrongNumber = 10;
	int correctNumber = 4;

	flcrm::log::log(flcrm::log::LogLevel::LEVEL_WARN, __FILE__, __LINE__, __FUNCTION__, "This number is wrong: {}, it should be this one: {}",wrongNumber,correctNumber);
	
	// you likely want to create macros for this.

#define LOG_ERROR(...)		flcrm::log::log(flcrm::log::LogLevel::LEVEL_TRACE, __FILE__, __LINE__, __FUNCTION__ ,__VA_ARGS__)

	LOG_ERROR("This number is wrong: {}, it should be this one: {}", wrongNumber, correctNumber);


	// ========================================
	//        !!!!! IMPORTANT !!!!!
	// ========================================
	// After logger is not longer needed shutdown() MUST be called explicitly to free devices stored in heap memory 
	flcrm::log::shutdown();

}


// ======== SETTING UP LOG CALLBACKS =========


class SomeClass {
public:
	void my_custom_log_callback_member_function(flcrm::log::LogMessage msg) {

		// you now have the formatted message and can do whatever you want, for example print it to a file

		std::string msg_has_the_string = msg.formated_message;
		flcrm::log::LogLevel msg_also_has_the_log_level = msg.log_level;
	}

};

void my_custom_log_callback_function(flcrm::log::LogMessage msg) {

	// you now have the formatted message and can do whatever you want, for example print it to a file

	std::string msg_has_the_string = msg.formated_message;
	flcrm::log::LogLevel msg_also_has_the_log_level = msg.log_level;
}

void callback_example() {


	// if you dont want to log to console and instead call some function in your code you can use the callback

	// create device for the callback
	flcrm::log::LogDevice callbackDevice;

	callbackDevice.output_type = flcrm::log::FUNCTION_CALLBACK; // set this to FUNCTION_CALLBACK now
	// ... configure device how you want in the same way as in the console example
	callbackDevice.format_info.add_source_function_name = false;

	SomeClass iHaveSomeClass;


	// the callback function must have this signature  void myFunc(flcrm::log::LogMessage& msg) -> std::function<void(LogMessage&)>
	// then use std::bind 

	callbackDevice.function_callback = std::bind(&SomeClass::my_custom_log_callback_member_function, iHaveSomeClass, std::placeholders::_1);
	callbackDevice.function_callback = std::bind(&my_custom_log_callback_function, std::placeholders::_1);


	// register device as normal

	flcrm::log::device_register(callbackDevice);

	// now logs will be formated and send to the callback function
	flcrm::log::log(flcrm::log::LogLevel::LEVEL_WARN, __FILE__, __LINE__, "" , "Log callback Message");



	// dont forget to shutdown when the programm ends!
	flcrm::log::shutdown();
}


*/




