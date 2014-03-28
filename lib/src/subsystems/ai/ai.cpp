#include "subsystems/ai/ai.h"

namespace game_engine
{
	namespace logic
	{
		namespace subsystems
		{
			namespace ai
			{
				void ai_subsystem::update_all()
				{
					for (auto& ent_vector : reg_entities)
					{
						for (auto ent_ptr : ent_vector.second)
						{
							auto& ent = static_cast<ai_entity&>(*ent_ptr);
							ent.ai_update();
						}
					}
				}
			}
		}
	}
}