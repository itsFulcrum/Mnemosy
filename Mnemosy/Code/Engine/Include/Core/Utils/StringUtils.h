#ifndef STRING_UTILS_H
#define STRING_UTILS_H

#include <vector>
#include <string>
#include <filesystem>
#include <utf8.h>

#include <codecvt>
#include <locale>


namespace mnemosy::core
{
	    
    class StringUtils
	{
	public:


		static std::string ToLowerCase(const std::string& str) {

			std::string lowerStr = str;
			std::transform(lowerStr.begin(), lowerStr.end(), lowerStr.begin(), [](unsigned char c) { return std::tolower(c); });
			return lowerStr;
		}

		static void MakeStringLowerCase(std::string& str) {
			std::transform(str.begin(), str.end(), str.begin(), [](unsigned char c) { return std::tolower(c); });
		}
		
		static void SortVectorListAlphabetcially(std::vector<std::string>& list){
			std::sort(list.begin(), list.end());
		}

		static bool string_is_valid_utf8(std::string& str) {
			return utf8::is_valid(str);

		}

		// if string is valid utf8 just returns it, otherwise first copies byte for byte into a wide string and then convert to a utf8string
		static std::string string_fix_u8Encoding(std::string& str) {


			if (utf8::is_valid(str.begin(), str.end())) {

				return str;
			}

			// convert byte by byte to wide String

			size_t length = str.length();
			std::wstring wStr;
			wStr.reserve(length);
			for (size_t i = 0; i < length; i++)
			{
				wStr.push_back(str[i] & 0xFF);
			}

			// convert back to u8string
			std::wstring_convert<std::codecvt_utf8<wchar_t>> utf8_conv;
			return utf8_conv.to_bytes(wStr);

		}

		static std::filesystem::path path_fix_u8Encoding(std::filesystem::path path) {

			std::string str = path.generic_string();

			if (utf8::is_valid(str.begin(), str.end())) {

				return path;
			}

			// convert byte by byte to wide String

			size_t length = str.length();
			std::wstring wStr;
			wStr.reserve(length);
			for (size_t i = 0; i < length; i++)
			{
				wStr.push_back(str[i] & 0xFF);
			}

			// convert back to u8string
			std::wstring_convert<std::codecvt_utf8<wchar_t>> utf8_conv;
			std::string u8String =  utf8_conv.to_bytes(wStr);

			return std::filesystem::path(u8String);
		}


		// wchar to utf8 string.  null terminator is added by this function
		static std::string wChar_to_utf8String(wchar_t w_char)
		{
			//const wchar_t arr[2]{ w_char,'\0' };
			std::wstring_convert<std::codecvt_utf8<wchar_t>> utf8_conv;
			return utf8_conv.to_bytes({ w_char,'\0' });
		}

		static std::string wChar_to_utf8String_appendable(wchar_t w_char)
		{
			//const wchar_t arr[2]{ w_char,'\0' };
			std::wstring_convert<std::codecvt_utf8<wchar_t>> utf8_conv;
			return utf8_conv.to_bytes({ w_char});
		}

		// wide string to utf8 encoded string
		static std::string wideString_to_utf8String(std::wstring& wide_string)
		{
			std::wstring_convert<std::codecvt_utf8<wchar_t>> utf8_conv;
			return utf8_conv.to_bytes(wide_string);
		}

		// utf8 endcoded string to wide string
		static std::wstring utf8String_to_wString(std::string& narrow_utf8_source_string)
		{
			std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
			return converter.from_bytes(narrow_utf8_source_string);
		}

		// utf8 encoded or not this will return a wide string if standart convert fails by just dropping the raw bytes to the string one by one.
		static std::wstring string_to_wString(const std::string& input)
		{
			try
			{
				std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
				return converter.from_bytes(input);
			}
			catch (std::range_error& e)
			{
				size_t length = input.length();
				std::wstring result;
				result.reserve(length);
				for (size_t i = 0; i < length; i++)
				{
					result.push_back(input[i] & 0xFF);
				}
				return result;
			}
		}


	};

}


#endif // ! STRING_UTILS_H