#pragma once

#include "render/mesh.h"
#include "spec_subsystem.h"
#include "subsystems/render/render_component.h"
#include "thr_queue/thread_api.h"
#include <atomic>
#include <set>

namespace game_engine {
namespace logic {
namespace subsystems {
namespace render {
/** \brief Class that represents a subsystem that renders graphics.
 * It sorts all render components so that state changes are minimized,
 * and renders everything through a separate thread.
 */
class render_subsystem : public util::specialized_subsystem< render_component >
{
public:
  render_subsystem( );
  ~render_subsystem( );

  /** \brief Takes ownership of the thread and creates a window where
   * everything is drawn.
   * It creates drawers for each entity, taking care of synchronization
   * with the tick thread, and waiting for changes to happen in the
   * entities.
   */
  void handle_events( );

  /** \brief Returns whether the main window is closed.
   * This can only happen when the game is exiting.
   */
  bool has_exited( ) const;

  /** \brief This member function is the other part of the drawing
   * procedure. It synchronizes with the drawing thread for updating
   * drawers. It is called by the tick thread every tick.
   */
  virtual void update_all( ) override final;

private:
  /** \brief Separated because it's a self-contained function that
   * as cluttering the handle_events() member function.
   * \return true if drawers were updated, false otherwise.
   */
  bool update_drawers_if_nec( );

private:
  std::atomic< bool > exited{ false };
  std::atomic< bool > update_thread_waiting;
  std::multiset< std::shared_ptr< const game_engine::render::mesh > > meshes;
  boost::mutex mt;
  boost::condition_variable cv;
};
}
}
}
}
