#pragma once

#include "compiler_util.h"

#include <unordered_map>
#include <vector>
#include <typeindex>

namespace game_engine
{
	namespace logic
	{
		class entity;
		class subsystem
		{
		private:
			/** \brief The percentage of entities that can be unsorted per type. */
			static const float sorting_limit;

			/** \brief Sorts the entities of a certain type if they are too unsorted.
			 * This is measured by comparing the percentage of unsorted entities to sorting_limit.
			 * \param tin Type of entities to sort if necessary.
			 */
			void sort_vector_if_necessary(const std::type_index& tin);

			//Used by game_engine::logic::game.
			friend class game;

			/** \brief Member function called by game_engine::logic::game when it has started a tick.
			 * It can be overriden if necessary.
			 */
			virtual void start_tick() {};

			/** \brief Member function called by game_engine::logic::game when it has finished a tick.
			* It can be overriden if necessary.
			*/
			virtual void finish_tick() {};
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