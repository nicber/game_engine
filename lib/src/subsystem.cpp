#include "component.h"
#include "private/sort_if_necessary.h"
#include "subsystems/time.h"
#include "world.h"

#include <algorithm>

namespace game_engine
{
	namespace logic
	{
		const float subsystem::sorting_limit = 0.2f;

		bool subsystem::add_component(component& ent)
		{
			if (!accepts(ent))
			{
				return false;
			}

			reg_components[typeid(ent)].push_back(&ent);
			++not_sorted_map[typeid(ent)];
			detail::sort_if_necessary(reg_components, not_sorted_map, typeid(ent), sorting_limit);

			after_addition(ent);
			return true;
		}

		bool subsystem::try_remove_component(component& ent)
		{
			auto& type_vector = reg_components[typeid(ent)];
			if (type_vector.size() == 0)
			{
				return false;
			}

			detail::sort_if_necessary(reg_components, not_sorted_map, typeid(ent), sorting_limit);
			auto not_sorted_number = not_sorted_map[typeid(ent)];

			auto ent_it = std::lower_bound(type_vector.begin(), type_vector.end() - not_sorted_number, &ent);

			if (ent_it == type_vector.end() - not_sorted_number)
			{
				ent_it = std::find(type_vector.end() - not_sorted_number, type_vector.end(), &ent);
			}

			if (ent_it == type_vector.cend())
			{
				return false;
			}

			type_vector.erase(ent_it);
			after_removal(ent);
			detail::sort_if_necessary(reg_components, not_sorted_map, typeid(ent), sorting_limit);
			return true;
		}

		void subsystem::remove_component(component& ent)
		{
			if (!try_remove_component(ent))
			{
				throw std::invalid_argument("Requested to remove an component that wasn't registered in the system");
			}
		}

		void subsystem::start_tick()
		{
			after_start_tick();
		}

		void subsystem::finish_tick()
		{
			after_finish_tick();
		}
	}
}