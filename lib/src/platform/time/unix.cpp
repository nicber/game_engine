#include <ctime>

#include "time_access.h"

namespace game_engine
{
	namespace logic
	{
		time read_absolute_time()
		{
			timespec tim;
			clock_gettime(CLOCK_MONOTONIC, &tim);
			auto result = tim.tv_sec * 1000000; // * 1 000 000 converts secs to usecs.
			result += tim.tv_nsec / 1000; // / 1 000 converts nanosecs to usecs.
			return{ time::type::platform, result };
		}

		namespace platform
		{
			time_type from_us(long long us) no_except
			{
				return us * 1000;
			}

			long long to_us(time_type tt) no_except
			{
				return tt / 1000;
			}
		}
	}
}