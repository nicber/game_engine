#pragma once
#include "subsystem.h"

#include <algorithm>

namespace game_engine
{
	namespace logic
	{
		template <typename T, typename... Args>
		void game::new_subsystem(Args&&... args)
		{
			new_subsystem(std::unique_ptr<T>(new T(std::forward<Args>(args)...)));
		}

		template <typename T>
		T& game::get() const
		{
			for (auto& ptr : subsystems)
			{
				T* res{ dynamic_cast<T*>(ptr.get())	};
				if (res)
				{
					return *res;
				}
			}

			throw no_subsystem_found<T>("Subsystem not found");
		}

		template <typename T>
		game::no_subsystem_found<T>::no_subsystem_found(std::string message):
			std::runtime_error(std::move(message))
		{}

		template <typename T>
		void game::remove_subsystem()
		{
			if (!try_remove_subsystem<T>())
			{
				throw no_subsystem_found<T>("Subsystem not found for removal");
			}
		}

		template <typename T>
		bool game::try_remove_subsystem()
		{
			auto& tid = typeid(T);
			auto it = std::find_if(subsystems.begin(), subsystems.end(),
				[&tid](std::unique_ptr<subsystem>& subsys_ptr){ 
          auto &subsys = *subsys_ptr;
          return tid == typeid(subsys);
        });

			if (it == subsystems.end())
			{
				return false;
			}
			
			subsystems.erase(it);
			return true;
		}
	}
}
