#pragma once

#include "component.h"
#include <memory>
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
          virtual std::unique_ptr<drawer> create_drawer() = 0;
				};
			}
		}
	}
}
