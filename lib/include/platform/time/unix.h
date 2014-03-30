#pragma once

#include <time.h>

namespace game_engine
{
	namespace logic
	{
		namespace platform
		{
			typedef decltype(timespec().tv_nsec) time_type;
		}
	}
}