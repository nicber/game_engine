#pragma once

#include "compiler_util.h"

#include "entity.h"

#include <unordered_map>
#include <memory>
#include <typeindex>
#include <vector>

namespace game_engine
{
	namespace logic
	{
		namespace detail
		{
			template <typename T>
			using entity_cont = std::unordered_map<std::type_index, std::vector<T>>;

			typedef std::unordered_map<std::type_index, size_t> sort_map;

			/** \brief Sorts the entities of a certain type if they are too unsorted.
			* This is measured by comparing the percentage of unsorted entities to sorting_limit.
			* \param tin Type of entities to sort if necessary.
			*/
			template <typename T>
			void sort_entities_if_necessary(entity_cont<T>& entities, sort_map& unsorted_number, std::type_index type, float sorting_limit) no_except;
		}
	}
}

#include "private/sort_entities_if_necessary.inl"