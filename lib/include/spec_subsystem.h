#pragma once

#include "subsystem.h"

namespace game_engine
{
	namespace logic
	{
		namespace util
		{
			/** \brief Utility template for subsystems that take only one entity type.
			 * It defines the accepts() member function so that it rejects any entity that is
			 * not a T.
			 */
			template <typename T>
			class specialized_subsystem : public subsystem
			{
			protected:
				/** \brief Only returns true if ent is of type T.
				 * \param ent The entity to filter.
				 * \return true if ent is of type T.
				 */
				virtual bool accepts(entity& ent) final override;
			};

#include "spec_subsystem.inl"
		}
	}
}