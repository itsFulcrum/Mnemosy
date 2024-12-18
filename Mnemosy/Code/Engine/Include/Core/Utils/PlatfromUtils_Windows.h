#ifndef PLATFORM_UTILS_WINDOWS_H
#define PLATFORM_UTILS_WINDOWS_H

#include <string>
#include <filesystem>


namespace mnemosy::core
{
	    
    class FileDialogs
	{
	public:

		// return empty path if it operation failed
		// filter can be like this ->     "png (*.png)\0*.png"
		static std::filesystem::path OpenFile(const char* filter);
		static std::filesystem::path SaveFile(const char* filter);

		static std::filesystem::path SelectFolder(const char* filter);
		
		// open a folder at specific path if path is valid,  return true on success
		static bool OpenFolderAt(const std::filesystem::path& folderPath);
	};

}

#endif // !PLATFORM_UTILS_WINDOWS_H
