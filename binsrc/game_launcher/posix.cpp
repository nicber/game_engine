#include "platform_specific.h"

#include <dlfcn.h>
#include <sstream>
#include <stdexcept>

namespace game_launcher
{
	namespace detail
	{
		find_lib_error open_library(const std::string& path, handle_t& result)
		{
			if ((result = dlopen(path.data(), 0)))
			{
				return no_find_func_error;
			}
			else
			{
				std::stringstream ss;
				ss << "Error loading library: " << dlerror();
				return ss.str();
			}
		}

		void release_library(const handle_t& handle)
		{
			if (!dlclose(handle))
			{
				std::stringstream ss;
				ss << "Unknown error when releasing library: " << dlerror();
				throw std::runtime_error(ss.str());
			}
		}

		find_func_error get_function(const std::string& name, const handle_t& handle, function_handle_t& result)
		{
			if ((result = dlsym(handle, name.data())))
			{
				return no_find_func_error;
			}
			else
			{
				std::stringstream ss;
				ss << "Error finding function named " << name << ": " << dlerror();
				return ss.str();
			}
		}
	}
}