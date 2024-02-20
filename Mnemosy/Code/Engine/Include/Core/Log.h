#ifndef LOG_H
#define LOG_H

#include "Engine/Include/MnemosyConfig.h"

#define _SILENCE_STDEXT_ARR_ITERS_DEPRECATION_WARNING
#define _SILENCE_ALL_MS_EXT_DEPRECATION_WARNINGS

#include <spdlog/spdlog.h>


#define MNEMOSY_LOGGER_NAME "MnemosyLogger"

#if defined MNEMOSY_PLATFORM_WINDOWS
	#define MNEMOSY_BREAK __debugbreak();
#elif defined MNEMOSY_PLATFORM_LINUX 
	#define MNEMOSY_BREAK __builtin_trap();

#endif // MNEMOSY_PLATFORM_WINDOWS




#ifndef MNEMOSY_CONFIG_RELEASE // On Debug

	#define MNEMOSY_TRACE(...)		if(spdlog::get(MNEMOSY_LOGGER_NAME) != nullptr) {spdlog::get(MNEMOSY_LOGGER_NAME)->trace(__VA_ARGS__);}
	#define MNEMOSY_DEBUG(...)		if(spdlog::get(MNEMOSY_LOGGER_NAME) != nullptr) {spdlog::get(MNEMOSY_LOGGER_NAME)->debug(__VA_ARGS__);}
	#define MNEMOSY_INFO(...)		if(spdlog::get(MNEMOSY_LOGGER_NAME) != nullptr) {spdlog::get(MNEMOSY_LOGGER_NAME)->info(__VA_ARGS__);}
	#define MNEMOSY_WARN(...)		if(spdlog::get(MNEMOSY_LOGGER_NAME) != nullptr) {spdlog::get(MNEMOSY_LOGGER_NAME)->warn(__VA_ARGS__);}
	#define MNEMOSY_ERROR(...)		if(spdlog::get(MNEMOSY_LOGGER_NAME) != nullptr) {spdlog::get(MNEMOSY_LOGGER_NAME)->error(__VA_ARGS__);}
	#define MNEMOSY_CRITICAL(...)	if(spdlog::get(MNEMOSY_LOGGER_NAME) != nullptr) {spdlog::get(MNEMOSY_LOGGER_NAME)->critical(__VA_ARGS__);}
	#define MNEMOSY_ASSERT(x, msg)	if((x)) {} else { MNEMOSY_CRITICAL("ASSERT - {}\n\t{}\n\tin file {}\n\ton line {}", #x, msg, __FILE__, __LINE__); MNEMOSY_BREAK}

#else // On Release

	#define MNEMOSY_TRACE(...)		(void)0
	#define MNEMOSY_DEBUG(...)		(void)0
	#define MNEMOSY_INFO(...)		if(spdlog::get(MNEMOSY_LOGGER_NAME) != nullptr) {spdlog::get(MNEMOSY_LOGGER_NAME)->info(__VA_ARGS__);}
	#define MNEMOSY_WARN(...)		if(spdlog::get(MNEMOSY_LOGGER_NAME) != nullptr) {spdlog::get(MNEMOSY_LOGGER_NAME)->warn(__VA_ARGS__);}
	#define MNEMOSY_ERROR(...)		if(spdlog::get(MNEMOSY_LOGGER_NAME) != nullptr) {spdlog::get(MNEMOSY_LOGGER_NAME)->error(__VA_ARGS__);}
	#define MNEMOSY_CRITICAL(...)	if(spdlog::get(MNEMOSY_LOGGER_NAME) != nullptr) {spdlog::get(MNEMOSY_LOGGER_NAME)->critical(__VA_ARGS__);}
	#define MNEMOSY_ASSERT(x, msg)	(void)0

#endif // !MNEMOSY_CONFIG_RELEASE

#endif // !Log_H
