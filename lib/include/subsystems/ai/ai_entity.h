#pragma once

#include "entity.h"
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
				class ai_entity : public logic::entity
				{
				public:
					/** \brief Called by ai_subsystem when it's updating all entities.
					 * It's where the code for updating an entity should be.
					 */
					virtual void ai_update() = 0;
				};
			}
		}
	}
}