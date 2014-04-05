#pragma once

#include "component.h"

#include <memory>
#include <typeindex>
#include <unordered_map>

namespace game_engine
{
	namespace logic
	{
		template <typename T, typename... Args>
		void entity::new_component(Args&&... args)
		{
			add_component(std::unique_ptr<component>(new T(std::forward<Args&&>(args)...)));
		}

		template <typename T>
		void entity::remove_component()
		{
			if (!try_remove_component<T>())
			{
				throw component_not_found<T>("Component not found.");
			}
		}

		template <typename T>
		bool entity::try_remove_component()
		{
			auto num = components.erase(typeid(T));
			return num > 0;
		}

		template <typename T>
		T& entity::get_comp() const
		{
			auto ptr = try_get_comp<T>();
			if (ptr)
			{
				return *ptr;
			} else {
				throw component_not_found<T>("Component not found.");
			}
		}

		template <typename T>
		T* entity::try_get_comp() const
		{
			auto it = components.find(typeid(T));
			return it != components.end() ? static_cast<T*>(it->second.get()) : nullptr;
		}
	}
}

#include "entity.inl"