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

		subsystem& component::get_subsys() const
		{
			if (subsys)
			{
				return *subsys;
			}
			throw std::runtime_error("Component not accepted by any entity.");
		}

		entity* component::try_get_parent_entity() const
		{
			return parent_ent;
		}

		game* component::try_get_parent_game() const
		{
			return parent_ent ? parent_ent->parent_game : nullptr;
		}

		subsystem* component::try_get_subsys() const
		{
			return subsys;
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