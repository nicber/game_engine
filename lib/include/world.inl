#pragma once
#include "subsystem.h"

namespace game_engine
{
	namespace logic
	{
		template <typename T, typename... Args>
		void game::new_subsystem(Args&&... args)
		{
			new_subsystem({ new T(std::forward<Args>(args)...) });
		}
	}
}