#pragma once

#include "compiler_util.h"
#include "time_access.h"

#include <unordered_map>
#include <vector>
#include <typeindex>

namespace game_engine
{
	namespace logic
	{
		class entity;
		class game;
		
		class subsystem
		{
		private:
			/** \brief A pointer to the game_engine::logic::game that owns this subsystem. */
			game* parent_game = nullptr;

			/** \brief The percentage of entities that can be unsorted per type. */
			static const float sorting_limit;

			/** \brief Member function called by game_engine::logic::game when it has started a tick.
			* It calls after_start_tick() which can be overriden by subclasses.
			*/
			void start_tick();

			/** \brief Member function called by game_engine::logic::game when it has finished a tick.
			* It calls after_finish_tick() which can be overriden by subclasses.
			*/
			void finish_tick();

			/* Used by game_engine::logic::game. */
			friend class game;
			friend void swap(game& lhs, game& rhs) no_except;
		protected:
			typedef std::unordered_map<std::type_index, std::vector<entity*>> entities_cont;
			typedef std::unordered_map<std::type_index, size_t> sort_map;
			entities_cont reg_entities;
			sort_map not_sorted_map;

			/** \brief Determines whether ent can be added to the subsystem.
			 * It has to be overriden by subclasses.
			 */
			virtual bool accepts(entity& ent) = 0;

			/** \brief Performs any post-addition process required by the subsystem.
			 * It can be overriden by the subsystem class.
			 */
			virtual void after_addition(entity&){}

			/** \brief Performs any post-removal process needed by the subsystem.
			 * It can be overriden by the subsystem class.
			 */
			virtual void after_removal(entity&){}

			/** \brief Called by start_tick() after it runs.
			 * It can be overriden by subclasses.
			 */
			virtual void after_start_tick() {}

			/** \brief Called by start_tick() after it runs.
			 * It can be overriden by subclasses.
		 	 */
			virtual void after_finish_tick() {}

			/** \brief Tries to remove an entity.
			 * \param ent Entity to remove.
			 * \return False if the entity wasn't in the system. True otherwise.
			 */
			bool try_remove_entity(entity& ent);
		public:
			/** \brief Adds an entity to the subsystem.
			 * It calls accepts() to see if the entity can be added.
			 * If it was accepted then it calls after_addition().
			 * \param ent The entity to add.
			 * \return true if it was accepted, false otherwise.
			 */
			bool add_entity(entity& ent);

			/** \brief Removes an entity from the subsystem.
			 * Calls after_removal() if it was registered in the subsystem.
			 * \param ent The entity to remove.
			 * \throw std::invalid_argument Thrown if the entity wasn't
			 * registered in the subsystem.
			 */
			void remove_entity(entity& ent);

			/** \brief Member function to update every entity registered in the subsystem.
			 * It has to be overriden by subclasses.
			 */
			virtual void update_all() = 0;

			/** \brief Needed for correct destruction of subclasses because they will be
			 * deleted from a subsystem*.
			 */
			virtual ~subsystem() {};
		};
	}
}