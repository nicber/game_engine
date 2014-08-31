#pragma once

#include <atomic>
#include "spec_subsystem.h"
#include "subsystems/render/render_component.h"
#include "thr_queue/thread_api.h"

namespace game_engine
{
	namespace logic
	{
		namespace subsystems
		{
			namespace render
			{
				/** \brief Class that represents a subsystem that renders graphics.
				 * It sorts all render components so that state changes are minimized,
         * and renders everything through a separate thread.
				 */
				class render_subsystem : public util::specialized_subsystem<render_component>
				{
				public:
          render_subsystem(std::atomic<bool> &exit_flag);
          ~render_subsystem();

					virtual void update_all() override final;

        private:
          void window_loop(std::atomic<bool> &exit_flag);

        private:
          boost::thread window_thr;
				};
			}
		}
	}
}
