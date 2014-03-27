#pragma once

#include "subsystems/ai/ai_entity.h"
#include "spec_subsystem.h"

namespace game_engine
{
	namespace logic
	{
		namespace subsystems
		{
			namespace ai
			{
				/** \brief Class that represents a subsystem that runs code defined by the user.
				 * It calls a member function in each entity called ai_update(). Entities have to subclass ai::entity to
				 * be accepted by the subsystem.
				 */
				class ai_subsystem : public util::specialized_subsystem<ai_entity>
				{
				public:
					virtual void update_all() override final;
				};
			}
		}
	}
}