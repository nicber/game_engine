#include "platform_specific.h"

#include <sstream>

namespace game_launcher
{
	namespace detail
	{
		find_lib_error open_library(const std::string& path, handle_t& result)
		{
			if (result = LoadLibrary(path.data()))
			{
				return no_find_func_error;
			}
			else
			{
				std::stringstream ss;
				ss << "Error loading library: " << GetLastError();
				return ss.str();
			}
		}

		void release_library(const handle_t& handle)
		{
			if (!FreeLibrary(handle))
			{
				std::stringstream ss;
				ss << "Unknown error when releasing library: " << GetLastError();
				throw std::runtime_error(ss.str());
			}
		}

		find_func_error get_function(const std::string& name, const handle_t& handle, function_handle_t& result)
		{
			if (result = GetProcAddress(handle, name.data()))
			{
				return no_find_func_error;
			}
			else
			{
				std::stringstream ss;
				ss << "Error finding function named " << name << ": " << GetLastError();
				return ss.str();
			}
		}
	}
}