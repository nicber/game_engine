#include "subsystem.h"

#include <algorithm>

namespace game_engine
{
	namespace logic
	{
		void subsystem::sort_vector_if_necessary(const std::type_index& tin)
		{
			auto& type_vector = reg_entities[tin];
			auto& not_sorted_number = not_sorted_map[tin];

			if (type_vector.size() == 0)
			{
				return;
			}

			if (float(not_sorted_number) / type_vector.size() > sorting_limit)
			{
				std::sort(type_vector.begin(), type_vector.end());
				not_sorted_number = 0;
			}
		}

		bool subsystem::add_entity(entity& ent)
		{
			if (!accepts(ent))
			{
				return false;
			}

			reg_entities[typeid(ent)].push_back(&ent);
			++not_sorted_map[typeid(ent)];
			sort_vector_if_necessary(typeid(ent));

			after_addition(ent);
			return true;
		}

		void subsystem::remove_entity(entity& ent)
		{
			auto& type_vector = reg_entities[typeid(ent)];
			if (type_vector.size() == 0)
			{
				throw std::invalid_argument("Requested to remove an entity that wasn't registered in the system");
			}

			sort_vector_if_necessary(typeid(ent));
			auto& not_sorted_number = not_sorted_map[typeid(ent)];

			auto ent_it = std::lower_bound(type_vector.cbegin(), type_vector.cend() - not_sorted_number, &ent);

			if (ent_it == type_vector.cend())
			{
				ent_it = std::find(type_vector.cend() - not_sorted_number, type_vector.cend(), &ent);
			}

			if (ent_it == type_vector.cend())
			{
				throw std::invalid_argument("Requested to remove an entity that wasn't registered in the system");
			}

			type_vector.erase(ent_it);
			after_removal(ent);
		}
	}
}