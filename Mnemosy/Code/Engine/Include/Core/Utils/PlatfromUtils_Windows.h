#ifndef PLATFORM_UTILS_WINDOWS_H
#define PLATFORM_UTILS_WINDOWS_H

#include <string>


namespace mnemosy::core
{

	class FileDialogs
	{
	public:
		static std::string OpenFile(const char* filter);
		static std::string SaveFile(const char* filter);
	};

}

#endif // !PLATFORM_UTILS_WINDOWS_H
