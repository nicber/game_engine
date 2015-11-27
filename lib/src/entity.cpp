#include "entity.h"
#include "world.h"

namespace game_engine
{
	namespace logic
	{
		game& entity::get_parent_game() const
		{
			if (parent_game)
			{
				return *parent_game;
			}
			throw std::runtime_error("Entity not added to any game.");
		}

		void entity::add_component(std::unique_ptr<component> comp_ptr)
		{
      auto &comp = *comp_ptr;
			auto& ptr = components[typeid(comp)];
			if (ptr)
			{
				throw component_exists("Component already exists in the entity.");
			}
			ptr = std::move(comp_ptr);
			ptr->parent_ent = this;

			if (parent_game)
			{
				parent_game->add_component(*ptr);
			}
		}

		entity::~entity()
		{
			if (parent_game)
			{
				parent_game->remove_entity(*this);
			}
		}
	}
}
