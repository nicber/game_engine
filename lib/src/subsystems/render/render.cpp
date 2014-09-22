#include <GL/glew.h>
#include <SFML/Window.hpp>
#include "subsystems/render/render.h"
#include "time_access.h"

namespace game_engine {
namespace logic {
namespace subsystems {
namespace render {
render_subsystem::render_subsystem()
  : update_thread_waiting(false)
{}

render_subsystem::~render_subsystem()
{}

/** \brief This member function
 */
void render_subsystem::handle_events() {
  const boost::chrono::milliseconds wait_time(5);

  sf::Window window(sf::VideoMode(800, 600), "My window");
  glewInit();
  glViewport(0, 0, 800, 600);

  time last_update_time = read_absolute_time();
  unsigned long long time_since_last_update = 0;

  while (true) {
    {
      boost::unique_lock<boost::mutex> lock(mt);
      if (!update_thread_waiting) {
        cv.wait_for(lock, wait_time, [this] {
          return (bool) update_thread_waiting;
        });
      }

      if (update_drawers_if_nec()) {
        last_update_time = read_absolute_time();
        time_since_last_update = 0;
      } else {
        time_since_last_update = read_absolute_time().to(time::type::usec)
                                - last_update_time.to(time::type::usec);
      }

      sf::Event event;
      window.pollEvent(event);

      if (event.type == sf::Event::Closed) {
        window.close();
        exited = true;
        update_thread_waiting = false;
        cv.notify_one();
        return;
      } else if (event.type == sf::Event::Resized) {
        glViewport(0, 0, event.size.width, event.size.height);
      }
    } // unlock the lock.

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   
    for (auto& drawer : drawers) {
      drawer->draw(time_since_last_update);
    }

    window.display();
  }
}

bool render_subsystem::has_exited() const {
  return exited;
}

void render_subsystem::update_all() {
  assert(!update_thread_waiting);
  boost::unique_lock<boost::mutex> lock(mt);
  if (exited) {
    return;
  }
  update_thread_waiting = true;
  cv.notify_one();
  cv.wait(lock, [&] { return !update_thread_waiting; });
}

bool render_subsystem::update_drawers_if_nec() {
  // Assumes that the mt mutex is locked by the calling thread.
  if (update_thread_waiting) {
    drawers.clear();
    for (auto& components_of_a_type : reg_components) {
      auto& vector_of_components = components_of_a_type.second;
      for (auto comp : vector_of_components) {
        auto drawer = static_cast<render_component*>(comp)->create_drawer();
        if (!drawers.size() || !drawers.back()->try_combine_with(*drawer)) {
          drawers.emplace_back(std::move(drawer));
        }
      }
    }

    update_thread_waiting = false;
    cv.notify_one();
    return true;
  }
  return false;
}
}
}
}
}
