#pragma once

#include <string>
#include <windows.h>

namespace game_launcher
{
	namespace detail
	{
		typedef HMODULE handle_t;
		typedef std::string find_lib_error;

		typedef FARPROC function_handle_t;
		typedef std::string find_func_error;
	}
}