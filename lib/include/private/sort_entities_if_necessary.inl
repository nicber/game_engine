#include <algorithm>

namespace game_engine
{
	namespace logic
	{
		namespace detail
		{
			template <typename T>
			void sort_entities_if_necessary(entity_cont<T>& entities, sort_map& unsorted_number, std::type_index type, float sorting_limit) no_except
			{
				auto& type_vector = entities[type];
				auto& not_sorted_number = unsorted_number[type];

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
		}
	}
}