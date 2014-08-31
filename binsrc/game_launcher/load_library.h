#pragma once

#include <iostream>
#include <functional>
#include <stdexcept>

#include "platform_specific.h"

namespace game_launcher
{
	class lib_not_found : public std::runtime_error
	{
		using std::runtime_error::runtime_error;
	};

	class library
	{
		detail::handle_t handle;

	public:
		/** \brief Tries to open a library on path.
		 * \param path The library path to open.
		 * \throws lib_not_found.
		 */
		library(const std::string& path);

		/** \brief Releases the library.
		 */
		~library();

		/** \brief Tries to get a function called func_name.
		 * \param func_name Function name to find.
		 * \return A callable std::function on success and a default-constructed std::function otherwise.
		 */
		template <typename Function>
		std::function<Function> get(const std::string& func_name) const
		{
			detail::function_handle_t func;
			detail::find_func_error error = detail::get_function(func_name, handle, func);
			if(error != detail::no_find_func_error) {
				std::cerr << error << '\n';
				return {};
			}
			return{ reinterpret_cast<Function*>(func) };
		}
	};
}