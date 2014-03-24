#include "world.h"

namespace game_engine
{
	namespace logic
	{
		void game::new_subsystem(std::unique_ptr<subsystem> subsys)
		{
			subsystems.emplace_back(std::move(subsys));
		}

		void game::tick()
		{
			for (auto& subsys_ptr : subsystems)
			{
				auto& subsys(*subsys_ptr);
				subsys.start_tick();
			}

			for (auto& subsys_ptr : subsystems)
			{
				auto& subsys(*subsys_ptr);
				subsys.update_all();
			}

			for (auto& subsys_ptr : subsystems)
			{
				auto& subsys(*subsys_ptr);
				subsys.finish_tick();
			}
		}
	}
}