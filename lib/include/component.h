#pragma once

#include <typeindex>

namespace game_engine
{
	namespace logic
	{
		class entity;
		class game;
		class subsystem;

		/** \brief Class that represents a generic component. It's meant
		 * to be subclassed.
		 */
		class component
		{
			friend class entity;
			friend class subsystem;
			friend class game;
			/** \brief Stores a pointer to the subsystem that accepted this component. */
			subsystem* subsys = nullptr;

			/** \brief Stores a pointer to the entity that owns this component. */
			entity* parent_ent = nullptr;
			
			/** \brief Stores the type of this component when it's fully
			 * constructed. This is needed so that when we remove ourselves
			 * from the subsystem, it will have a way of knowing what type
			 * this entity was and where it should look for it.
			 */
			mutable std::type_index typ = typeid(component);

			std::type_index tid() const;
		public:
			/** \brief Returns a reference to the game that owns the entity that owns
			 * this component. If there isn't one, it throws std::runtime_error.
			 */
			game& get_parent_game() const;

			/** \brief Returns a reference to the entity that owns this
			 * component. Throws std::runtime_error if there isn't any.
			 */
			entity& get_parent_ent() const;

			/** \brief Returns a reference to the subsystem that accepted this
			 * component. Throws std::runtime_error if there isn't any.
			 */
			subsystem& get_subsys() const;

			/** \brief Tries to return the parent game.
			 * Returns nullptr if there isn't any or this component isn't owned by any entity.
			 */
			game*  try_get_parent_game() const;

			/** \brief Tries to return the parent game.
			* Returns nullptr if there isn't any.
			*/
			entity*  try_get_parent_entity() const;

			/** \brief Tries to return the subsystem that accepted this component.
			* Returns nullptr if there isn't any.
			*/
			subsystem*  try_get_subsys() const;

			virtual ~component();
		};
	}
}