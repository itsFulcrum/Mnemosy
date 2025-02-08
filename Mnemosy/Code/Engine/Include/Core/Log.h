#ifndef LOG_H
#define LOG_H

#include "Include/MnemosyConfig.h"

#include <FulcrumUtils/Flcrm_Log.hpp>

#define _SILENCE_STDEXT_ARR_ITERS_DEPRECATION_WARNING
#define _SILENCE_ALL_MS_EXT_DEPRECATION_WARNINGS

#if defined MNEMOSY_PLATFORM_WINDOWS

	#define MNEMOSY_BREAK __debugbreak();

#elif defined MNEMOSY_PLATFORM_LINUX 
	#define MNEMOSY_BREAK __builtin_trap();

#endif // MNEMOSY_PLATFORM_WINDOWS



#ifndef FUNCTION_NAME
#ifdef WIN32   //WINDOWS
#define FUNCTION_NAME   __FUNCTION__  
#else          //Linux
#define FUNCTION_NAME   __func__ 
#endif
#endif


#define MNEMOSY_TRACE(...)		flcrm::log::log(flcrm::log::LogLevel::LEVEL_TRACE, __FILE__, __LINE__, FUNCTION_NAME,__VA_ARGS__)
#define MNEMOSY_DEBUG(...)		flcrm::log::log(flcrm::log::LogLevel::LEVEL_DEBUG, __FILE__, __LINE__, FUNCTION_NAME,__VA_ARGS__)
#define MNEMOSY_INFO(...)		flcrm::log::log(flcrm::log::LogLevel::LEVEL_INFO , __FILE__, __LINE__, FUNCTION_NAME,__VA_ARGS__)
#define MNEMOSY_WARN(...)		flcrm::log::log(flcrm::log::LogLevel::LEVEL_WARN , __FILE__, __LINE__, FUNCTION_NAME,__VA_ARGS__)
#define MNEMOSY_ERROR(...)		flcrm::log::log(flcrm::log::LogLevel::LEVEL_ERROR, __FILE__, __LINE__, FUNCTION_NAME,__VA_ARGS__)
#define MNEMOSY_CRITICAL(...)	flcrm::log::log(flcrm::log::LogLevel::LEVEL_FATAL, __FILE__, __LINE__, FUNCTION_NAME,__VA_ARGS__)

#define MNEMOSY_ASSERT(x, msg)	if((x)) {} else { MNEMOSY_CRITICAL("ASSERT - {}\n\t{}\n\tin file {}\n\ton line {}", #x, msg, __FILE__, __LINE__); MNEMOSY_BREAK}


#endif // !Log_H
