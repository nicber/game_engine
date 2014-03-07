#include "subsystem.h"

namespace game_engine
{
	namespace logic
	{
		bool subsystem::add_entity(entity& ent)
		{
			if (!accepts(ent))
			{
				return false;
			}

			reg_entities.push_back(&ent);
			after_addition(ent);
			return true;
		}

		void subsystem::remove_entity(entity& ent)
		{
			auto ent_it = std::find(reg_entities.cbegin(), reg_entities.cend(), &ent);
			if (ent_it == reg_entities.cend())
			{
				throw std::invalid_argument("Requested to remove an entity that wasn't registered in the system");
			}

			reg_entities.erase(ent_it);
			after_removal(ent);
		}
	}
}