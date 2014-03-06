#include "load_library.h"

namespace game_launcher
{
	using namespace detail;
	library::library(const std::string& path)
	{
		auto open_err(open_library(path, handle));
		if (no_find_lib_error != open_err)
		{
			throw std::invalid_argument(open_err);
		}
	}

	library::~library()
	{
		release_library(handle);
	}
}