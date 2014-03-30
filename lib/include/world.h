#pragma once

#include "compiler_util.h"

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
		class subsystem;

		/** \brief Class responsible for running a game.
		 * Subsystems can be added to it.
		 */
		class game
		{
			std::list<std::unique_ptr<subsystem>> subsystems;
			
			typedef std::unordered_map<std::type_index, std::vector<std::unique_ptr<entity>>> entity_cont;
			typedef std::unordered_map<std::type_index, size_t> sort_map;

			/** \brief A map that stores entities in vectors according to their type. */
			entity_cont entities;

			/** \brief A map that stores how many unsorted entities there are
			 * at the end of each vector.
			 */
			sort_map unsorted_number;

			/** \brief Indicates a maximum percentage of entities that can be
			 * unsorted in a vector.
			 */
			static const float max_unsorted_percentage;
		public:
			friend void swap(game& lhs, game& rhs) no_except;

			/** \brief Default constructor. */
			game() = default;

			/** \brief Move constructor. */
			game(game&& other);
			
			/** \brief Move assignment operator */
			game& operator=(game&& rhs) no_except;

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

			/** \brief Tries to add an entity to all subsystems. 
			 * \return True it it has been added to at least one.
			 */
			bool add_entity(std::unique_ptr<entity> ent);
		};
	}
}

#include "world.inl"