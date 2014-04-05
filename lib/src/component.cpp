#include "component.h"

#include "entity.h"
#include "subsystem.h"

namespace game_engine
{
	namespace logic
	{
		entity& component::get_parent_ent() const
		{
			if (parent_ent)
			{
				return *parent_ent;
			}
			throw std::runtime_error("Component not added to any entity.");
		}

		game& component::get_parent_game() const
		{
			return get_parent_ent().get_parent_game();
		}

		component::~component()
		{
			if (subsys)
			{
				subsys->remove_component(*this);
			}
		}
	}
}