#ifndef STRING_UTILS_H
#define STRING_UTILS_H


#include <vector>
#include <string>

namespace mnemosy::core
{
	    
    class StringUtils
	{
	public:

		static void MakeStringLowerCase(std::string& str)
		{
			std::transform(str.begin(), str.end(), str.begin(), [](unsigned char c) { return std::tolower(c); });
		}
		
		static void SortVectorListAlphabetcially(std::vector<std::string>& list){

			//std::vector<std::string> sorted = list;
			std::sort(list.begin(), list.end());

			//return sorted;
		}

	};

}


#endif // ! STRING_UTILS_H