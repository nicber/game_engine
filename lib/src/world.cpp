#include "world.h"

namespace game_engine
{
	namespace logic
	{
		void swap(game& lhs, game& rhs) no_except
		{
			using std::swap;
			swap(lhs.subsystems, rhs.subsystems);

			/*
			 * subsystem.parent_game has to point to the game that owns the subsystem.
			 * After swapping the subsystems, their elements have parent_games that are
			 * pointing to their previous game.
			 */

			for (auto& subsys_ptr : lhs.subsystems)
			{
				subsys_ptr->parent_game = &lhs;
			}

			for (auto& subsys_ptr : rhs.subsystems)
			{
				subsys_ptr->parent_game = &rhs;
			}
		}

		game::game(game&& other):
		game()
		{
			using std::swap;
			swap(*this, other);
		}

		game& game::operator=(game&& rhs) no_except
		{
			using std::swap;
			this->~game();
			new (this) game();
			swap(*this, rhs);
			return *this;
		}

		void game::new_subsystem(std::unique_ptr<subsystem> subsys)
		{
			subsystems.emplace_back(std::move(subsys));
			subsystems.back()->parent_game = this;
		}

		void game::tick()
		{
			for (auto& subsys_ptr : subsystems)
			{
				subsys_ptr->start_tick();
			}

			for (auto& subsys_ptr : subsystems)
			{
				subsys_ptr->update_all();
			}

			for (auto& subsys_ptr : subsystems)
			{
				subsys_ptr->finish_tick();
			}
		}
	}
}