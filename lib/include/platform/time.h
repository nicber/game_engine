#pragma once

#include "compiler_util.h"

#ifdef _WIN32
#include "platform/time/win32.h"
#endif

#ifdef __unix__
#include "platform/time/unix.h"
#endif

namespace game_engine
{
	namespace logic
	{
		namespace platform
		{
			/** \brief Converts from the platform specific time type to microseconds. */
			time_type from_us(long long us) no_except;

			/** \brief Converts from microseconds to the platform specific time type. */
			long long to_us(time_type tt) no_except;
		}
	}
}
