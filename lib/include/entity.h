#pragma once

#include "world.h"

namespace game_engine
{
	namespace logic
	{
		/** \brief Basic class that is the base class of all entities. */
		class entity
		{
			friend class game;
			/** \brief Stores a pointer to the game that owns this entity. */
			game* parent_game;
		protected:
			/** \brief Returns the game that owns the this entity. */
			game& get_parent_game() const;
		public:
			virtual ~entity() {};
		};
	}
}