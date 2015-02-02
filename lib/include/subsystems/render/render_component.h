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
          : public virtual logic::component
				{
				public:
          virtual std::shared_ptr<const drawer> update_opengl_get_drawer() = 0;
				};
			}
		}
	}
}
