#pragma once

#include "subsystem.h"

namespace game_engine
{
	namespace logic
	{
		namespace util
		{
			/** \brief Utility template for subsystems that take only one component type.
			 * It defines the accepts() member function so that it rejects any component that is
			 * not a T.
			 */
			template <typename T>
			class specialized_subsystem : public subsystem
			{
			protected:
				/** \brief Only returns true if ent is of type T.
				 * \param ent The component to filter.
				 * \return true if ent is of type T.
				 */
				virtual bool accepts(component& comp) final override;
			};
		}
	}
}

#include "spec_subsystem.inl"