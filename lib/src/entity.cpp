#include "entity.h"

namespace game_engine
{
	namespace logic
	{
		game& entity::get_parent_game() const
		{
			return *parent_game;
		}
	}
}