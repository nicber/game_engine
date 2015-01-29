#pragma once

#include "component.h"
#include "world.h"

namespace game_engine
{
	namespace logic
	{
		namespace subsystems
		{
			namespace ai
			{
				/** \brief Class that represents an 'intelligent' class that can update itself. */
				class ai_component : public virtual logic::component
				{
				public:
					/** \brief Called by ai_subsystem when it's updating all components.
					 * It's where the code for updating a component should be.
					 */
					virtual void ai_update() = 0;
				};
			}
		}
	}
}
