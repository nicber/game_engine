#pragma once

#include <deque>
#include <typeinfo>

namespace game_engine
{
	namespace logic
	{
		class entity;
		class subsystem
		{
		protected:
			std::deque<entity*> reg_entities;

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
		};
	}
}