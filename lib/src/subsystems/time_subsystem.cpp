#include "subsystems/time_subsystem.h"

namespace game_engine
{
	namespace logic
	{
		namespace subsystems
		{
			void time_subsystem::after_start_tick()
			{
				//TODO.
			}

			milliseconds time_subsystem::absolute() const
			{
				return current_time;
			}
		}
	}
}