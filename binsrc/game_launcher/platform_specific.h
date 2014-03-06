#pragma once

#ifdef _WIN32
#include "win32.h"
#endif

namespace game_launcher
{
	namespace detail
	{
		const find_lib_error no_find_lib_error;
		const find_func_error no_find_func_error;

		/** \brief Tries to open a library.
		 * \param path The path to open.
		 * \param result Stores the result.
		 * \return An error if it fails. It can be compared to no_find_lib_error
		 *         to see if it failed.
		 */
		find_lib_error open_library(const std::string& path, handle_t& result);

		/** \brief Closes the library.
		 * \param handle The handle to release.
		 */
		void release_library(const handle_t& handle);

		/** \brief Tries to find a function named according to name.
		 * \param name The name to find.
		 * \param handle The library in which to search.
		 * \param result The found function.
		 * \return The error on failure, it can be compared to no_find_func_error
		 *         to see if it failed.
		 */
		find_func_error get_function(const std::string& name, const handle_t& handle, function_handle_t& result);
	}
}