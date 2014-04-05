#pragma once

#include "subsystems/ai/ai_component.h"
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
				 * It calls a member function in each component called ai_update(). Components have to subclass ai::component to
				 * be accepted by the subsystem.
				 */
				class ai_subsystem : public util::specialized_subsystem<ai_component>
				{
				public:
					virtual void update_all() override final;
				};
			}
		}
	}
}