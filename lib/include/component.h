#pragma once

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
			/** \brief Stores a pointer to the subsystem that accepted this component. */
			subsystem* subsys = nullptr;

			/** \brief Stores a pointer to the entity that owns this component. */
			entity* parent_ent = nullptr;
		protected:
			/** \brief Returns a reference to the game that owns the entity that owns
			 * this component. If there isn't one, it throws std::runtime_error.
			 */
			game& get_parent_game() const;

			/** \brief Returns a reference to the entity that owns this
			 * component. Throws std::runtime_error if there isn't any.
			 */
			entity& get_parent_ent() const;
		public:
			virtual ~component();
		};
	}
}