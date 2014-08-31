#pragma once

#include "boost/operators.hpp"
#include "component.h"

namespace game_engine
{
	namespace logic
	{
		namespace subsystems
		{
			namespace render
			{
				/** \brief Class that can draw itself. */
				class render_component
          : public logic::component
          , public boost::totally_ordered<render_component>
				{
				public:
          bool operator<(render_component &rhs) const;
          bool operator==(render_component &rhs) const;
				};
			}
		}
	}
}
