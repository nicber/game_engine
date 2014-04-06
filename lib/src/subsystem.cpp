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

		bool subsystem::add_component(component& comp)
		{
			auto tid = comp.tid();
			if (!accepts(comp))
			{
				return false;
			}

			reg_components[tid].push_back(&comp);
			++not_sorted_map[tid];
			detail::sort_if_necessary(reg_components, not_sorted_map, tid, sorting_limit);

			after_addition(comp);
			return true;
		}

		bool subsystem::try_remove_component(component& comp)
		{
			auto tid = comp.tid();
			auto& type_vector = reg_components[tid];
			if (type_vector.size() == 0)
			{
				return false;
			}

			detail::sort_if_necessary(reg_components, not_sorted_map, tid, sorting_limit);
			auto not_sorted_number = not_sorted_map[tid];

			auto comp_it = std::lower_bound(type_vector.begin(), type_vector.end() - not_sorted_number, &comp);

			if (comp_it == type_vector.end() - not_sorted_number)
			{
				comp_it = std::find(type_vector.end() - not_sorted_number, type_vector.end(), &comp);
			}

			if (comp_it == type_vector.cend())
			{
				return false;
			}

			type_vector.erase(comp_it);
			after_removal(comp);
			detail::sort_if_necessary(reg_components, not_sorted_map, tid, sorting_limit);
			return true;
		}

		void subsystem::remove_component(component& comp)
		{
			if (!try_remove_component(comp))
			{
				throw std::invalid_argument("Requested to remove a component that wasn't registered in the system");
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

		subsystem::~subsystem()
		{
			for (auto& comp_type_and_vector : reg_components)
			{
				auto& comp_vector = comp_type_and_vector.second;
				for (auto& comp : comp_vector)
				{
					comp->subsys = nullptr;
					after_removal(*comp);
				}
			}
		}
	}
}