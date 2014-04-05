#pragma once

#include "compiler_util.h"

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
			using cont = std::unordered_map<std::type_index, std::vector<T>>;

			typedef std::unordered_map<std::type_index, size_t> sort_map;

			/** \brief Sorts the Ts of a certain type if they are too unsorted.
			* This is measured by comparing the percentage of unsorted components to sorting_limit.
			* \param tin Type of components to sort if necessary.
			*/
			template <typename T>
			void sort_if_necessary(cont<T>& contained, sort_map& unsorted_number, std::type_index type, float sorting_limit) no_except;
		}
	}
}

#include "private/sort_if_necessary.inl"