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
		class component;
		class game;
		
		class subsystem
		{
		private:
			/** \brief A pointer to the game_engine::logic::game that owns this subsystem. */
			game* parent_game = nullptr;

			/** \brief The percentage of components that can be unsorted per type. */
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
			typedef std::unordered_map<std::type_index, std::vector<component*>> component_cont;
			typedef std::unordered_map<std::type_index, size_t> sort_map;
			component_cont reg_components;
			sort_map not_sorted_map;

			/** \brief Determines whether comp can be added to the subsystem.
			 * It has to be overriden by subclasses.
			 */
			virtual bool accepts(component& comp) = 0;

			/** \brief Performs any post-addition process required by the subsystem.
			 * It can be overriden by the subsystem class.
			 */
			virtual void after_addition(component&){}

			/** \brief Performs any post-removal process needed by the subsystem.
			 * It can be overriden by the subsystem class.
			 */
			virtual void after_removal(component&){}

			/** \brief Called by start_tick() after it runs.
			 * It can be overriden by subclasses.
			 */
			virtual void after_start_tick() {}

			/** \brief Called by start_tick() after it runs.
			 * It can be overriden by subclasses.
		 	 */
			virtual void after_finish_tick() {}

			/** \brief Tries to remove an component.
			 * \param comp Component to remove.
			 * \return False if the component wasn't in the system. True otherwise.
			 */
			bool try_remove_component(component& comp);
		public:
      /** \brief Default constructor. */
      subsystem();

      /** \brief Subsystems shouldn't be copyable or movable. */
      subsystem(subsystem&) = delete;
      subsystem(subsystem&&) = delete;

      subsystem &operator=(subsystem&) = delete;
      subsystem &operator=(subsystem&&) = delete;

			/** \brief Adds an component to the subsystem.
			 * It calls accepts() to see if the component can be added.
			 * If it was accepted then it calls after_addition().
			 * \param comp The component to add.
			 * \return true if it was accepted, false otherwise.
			 */
			bool add_component(component& comp);

			/** \brief Removes an component from the subsystem.
			 * Calls after_removal() if it was registered in the subsystem.
			 * \param comp The component to remove.
			 * \throw std::invalid_argument Thrown if the component wasn't
			 * registered in the subsystem.
			 */
			void remove_component(component& comp);

			/** \brief Member function to update every component registered in the subsystem.
			 * It has to be overriden by subclasses.
			 */
			virtual void update_all() = 0;

			/** \brief Needed for correct destruction of subclasses because they will be
			 * deleted from a subsystem*.
			 */
			virtual ~subsystem();
		};
	}
}
