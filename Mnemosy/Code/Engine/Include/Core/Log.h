#ifndef LOG_H
#define LOG_H

#include "Include/MnemosyConfig.h"

#define _SILENCE_STDEXT_ARR_ITERS_DEPRECATION_WARNING
#define _SILENCE_ALL_MS_EXT_DEPRECATION_WARNINGS

#include <spdlog/spdlog.h>

#define MNEMOSY_LOGGER_NAME "MnemosyLogger"

#if defined MNEMOSY_PLATFORM_WINDOWS
	#define MNEMOSY_BREAK __debugbreak();
#elif defined MNEMOSY_PLATFORM_LINUX 
	#define MNEMOSY_BREAK __builtin_trap();

#endif // MNEMOSY_PLATFORM_WINDOWS


#include <FulcrumUtils/Flcrm_Log.hpp>

namespace mnemosy::core {

#ifdef MNEMOSY_CONFIG_DEBUG
	static flcrm::log::LogFormatInfo MNSY_log_console_format_info{false,false, false, false, false, false, false, false, true, false, true, true};
#endif // MNEMOSY_DEBUG

#ifdef MNEMOSY_CONFIG_RELEASE
	static flcrm::log::LogFormatInfo MNSY_log_console_format_info{false,false, false, false, false, false, false, false, true, false, false, false};
#endif // MNEMOSY_CONFIG_RELEASE


}


#ifndef FUNCTION_NAME
#ifdef WIN32   //WINDOWS
#define FUNCTION_NAME   __FUNCTION__  
#else          //Linux
#define FUNCTION_NAME   __func__ 
#endif
#endif



//#ifndef MNEMOSY_CONFIG_RELEASE // On Debug
	

#define MNEMOSY_TRACE(...)		flcrm::log::log_print(flcrm::log::flcrm_log_level::FLCRM_LOG_LEVEL_TRACE,true,__FILE__, FUNCTION_NAME,__LINE__,mnemosy::core::MNSY_log_console_format_info,__VA_ARGS__)
#define MNEMOSY_DEBUG(...)		flcrm::log::log_print(flcrm::log::flcrm_log_level::FLCRM_LOG_LEVEL_DEBUG,true,__FILE__, FUNCTION_NAME,__LINE__,mnemosy::core::MNSY_log_console_format_info,__VA_ARGS__)
#define MNEMOSY_INFO(...)		flcrm::log::log_print(flcrm::log::flcrm_log_level::FLCRM_LOG_LEVEL_INFO ,true,__FILE__, FUNCTION_NAME,__LINE__,mnemosy::core::MNSY_log_console_format_info,__VA_ARGS__)
#define MNEMOSY_WARN(...)		flcrm::log::log_print(flcrm::log::flcrm_log_level::FLCRM_LOG_LEVEL_WARN ,true,__FILE__, FUNCTION_NAME,__LINE__,mnemosy::core::MNSY_log_console_format_info,__VA_ARGS__)
#define MNEMOSY_ERROR(...)		flcrm::log::log_print(flcrm::log::flcrm_log_level::FLCRM_LOG_LEVEL_ERROR,true,__FILE__, FUNCTION_NAME,__LINE__,mnemosy::core::MNSY_log_console_format_info,__VA_ARGS__)
#define MNEMOSY_CRITICAL(...)	flcrm::log::log_print(flcrm::log::flcrm_log_level::FLCRM_LOG_LEVEL_FATAL,true,__FILE__, FUNCTION_NAME,__LINE__,mnemosy::core::MNSY_log_console_format_info,__VA_ARGS__)


	//#define MNEMOSY_TRACE(...)		if(spdlog::get(MNEMOSY_LOGGER_NAME) != nullptr) {spdlog::get(MNEMOSY_LOGGER_NAME)->trace(__VA_ARGS__);}
	//#define MNEMOSY_DEBUG(...)		if(spdlog::get(MNEMOSY_LOGGER_NAME) != nullptr) {spdlog::get(MNEMOSY_LOGGER_NAME)->debug(__VA_ARGS__);}
	//#define MNEMOSY_INFO(...)		if(spdlog::get(MNEMOSY_LOGGER_NAME) != nullptr) {spdlog::get(MNEMOSY_LOGGER_NAME)->info(__VA_ARGS__);}
	//#define MNEMOSY_WARN(...)		if(spdlog::get(MNEMOSY_LOGGER_NAME) != nullptr) {spdlog::get(MNEMOSY_LOGGER_NAME)->warn(__VA_ARGS__);}
	//#define MNEMOSY_ERROR(...)		if(spdlog::get(MNEMOSY_LOGGER_NAME) != nullptr) {spdlog::get(MNEMOSY_LOGGER_NAME)->error(__VA_ARGS__);}
	//#define MNEMOSY_CRITICAL(...)	if(spdlog::get(MNEMOSY_LOGGER_NAME) != nullptr) {spdlog::get(MNEMOSY_LOGGER_NAME)->critical(__VA_ARGS__);}
	#define MNEMOSY_ASSERT(x, msg)	if((x)) {} else { MNEMOSY_CRITICAL("ASSERT - {}\n\t{}\n\tin file {}\n\ton line {}", #x, msg, __FILE__, __LINE__); MNEMOSY_BREAK}

//#else // On Release

	/*#define MNEMOSY_TRACE(...)		(void)0
	#define MNEMOSY_DEBUG(...)		(void)0
	#define MNEMOSY_INFO(...)		if(spdlog::get(MNEMOSY_LOGGER_NAME) != nullptr) {spdlog::get(MNEMOSY_LOGGER_NAME)->info(__VA_ARGS__);}
	#define MNEMOSY_WARN(...)		if(spdlog::get(MNEMOSY_LOGGER_NAME) != nullptr) {spdlog::get(MNEMOSY_LOGGER_NAME)->warn(__VA_ARGS__);}
	#define MNEMOSY_ERROR(...)		if(spdlog::get(MNEMOSY_LOGGER_NAME) != nullptr) {spdlog::get(MNEMOSY_LOGGER_NAME)->error(__VA_ARGS__);}
	#define MNEMOSY_CRITICAL(...)	if(spdlog::get(MNEMOSY_LOGGER_NAME) != nullptr) {spdlog::get(MNEMOSY_LOGGER_NAME)->critical(__VA_ARGS__);}
	#define MNEMOSY_ASSERT(x, msg)	(void)0*/

//#endif // !MNEMOSY_CONFIG_RELEASE





#endif // !Log_H
