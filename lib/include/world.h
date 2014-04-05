#pragma once

#include "compiler_util.h"
#include "entity.h"

#include <list>
#include <unordered_map>
#include <memory>
#include <stdexcept>
#include <string>
#include <typeindex>
#include <vector>

namespace game_engine
{
	namespace logic
	{
		class entity;
		class component;
		class subsystem;

		/** \brief Class responsible for running a game.
		 * Subsystems can be added to it.
		 */
		class game
		{
			std::list<std::unique_ptr<subsystem>> subsystems;
			
			typedef std::unordered_map<std::type_index, std::vector<std::unique_ptr<entity>>> entities_cont;
			typedef std::unordered_map<std::type_index, size_t> sort_map;

			/** \brief Stores whether the destructor of game has been called.
			 * It's useful to know that because then we might be called by
			 * entities or other things that were part of the game and
			 * now are being destroyed. These objects can call member
			 * functions for informing that they are no longer part of the game.
			 * For making sure we don't store dangling pointers we remove them,
			 * but this implies modifing already destroyed objects (eg. entities).
			 */
			bool being_destroyed = false;

			/** \brief A map that stores entities in vectors according to their type. */
			entities_cont entities;

			/** \brief A map that stores how many unsorted components there are
			 * at the end of each vector.
			 */
			sort_map unsorted_number;

			/** \brief Indicates a maximum percentage of components that can be
			 * unsorted in a vector.
			 */
			static const float max_unsorted_percentage;

			friend class entity;
			/** \brief Adds a component to the game.
			* It is added to the first subsystem that accepts it.
			* Returns false if it wasn't accepted by any and true otherwise.
			*/
			bool add_component(component& comp);
		public:
			friend void swap(game& lhs, game& rhs) no_except;

			/** \brief Default constructor. */
			game() = default;

			/** \brief Move constructor. */
			game(game&& other);
			
			/** \brief Move assignment operator */
			game& operator=(game&& rhs) no_except;

			~game();

			/** \brief Adds a new subsystem to the engine.
			 * \param subsys A std::unique_ptr pointing to the subsystem to add.
			 */
			void new_subsystem(std::unique_ptr<subsystem> subsys);

			/** \brief Adds a new subsystem to the engine.
			 * It can directly be constructed directly by the engine for performance.
			 * \param args Paramenters for the constructor of T.
			 */
			template <typename T, typename... Args>
			void new_subsystem(Args&&... args);

			/** \brief Updates every subsystem in the game.
			 * If a subsystem that measures time has been added other
			 * subsystems can call ms_since_last_tick() to get the delta time
			 * since the last time they were updated, and absolute_time() to get
			 * the time in ms since the game started.
			 */
			void tick();

			/** \brief Gets the subsystem of type T if it's in this game. Otherwise
			 * it throws no_subsystem_found<T> if it's not in the game.
			 * \throw no_subsystem_found<T>
			 */
			template <typename T>
			T& get() const;

			/** \brief Class that represents an exception thrown by get<T>() when it can't find
			 * a subsystem.
			 */
			template <typename T>
			struct no_subsystem_found : public std::runtime_error
			{
				no_subsystem_found(std::string message);
			};

			/** \brief Adds an entity to the game.
			 * Adds all its components to all subsystems.
			 */
			void add_entity(std::unique_ptr<entity> ent);

			/** \brief Removes entity from the game.
			 * \throw std::invalid_argument if it wasn't registered.
			 */
			void remove_entity(entity& ent);
		};
	}
}

#include "world.inl"