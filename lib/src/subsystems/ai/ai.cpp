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
					for (auto& comp_vector : reg_components)
					{
						for (auto comp_ptr : comp_vector.second)
						{
							auto& ent = dynamic_cast<ai_component&>(*comp_ptr);
							ent.ai_update();
						}
					}
				}
			}
		}
	}
}
