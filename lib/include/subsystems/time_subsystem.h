#pragma once

#include "subsystem.h"

namespace game_engine
{
	namespace logic
	{
		namespace subsystems
		{
			/** \brief Class that represents a subsystem that can inform the time.
			 * It should be added to a game so other subsystems can know the time delta
			 * between different ticks.
			 */
			class time_subsystem : public subsystem
			{
				/** \brief Updated after the call to after_start_tick().
				 * It is returned by absolute().
				 */
				milliseconds current_time = 0;

				/** \brief Updates the internal clock. */
				void after_start_tick() override;
			public:
				/** \brief Returns the time in milliseconds. */
				milliseconds absolute() const;
			};
		}
	}
}