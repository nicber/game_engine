#pragma once
#include "subsystem.h"

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
	}
}