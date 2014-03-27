#pragma once

#include <string>

namespace game_launcher
{
	namespace detail
	{
		typedef void* handle_t;
		typedef std::string find_lib_error;

		typedef void* function_handle_t;
		typedef std::string find_func_error;
	}
}