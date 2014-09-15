#pragma once

#include "component.h"
#include "subsystems/render/drawer.h"

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
				{
				public:
          drawer create_drawer();
				};
			}
		}
	}
}
