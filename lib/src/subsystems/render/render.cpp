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

void render_subsystem::handle_events() {
  const boost::chrono::milliseconds wait_time(5);

  sf::Window window(sf::VideoMode(800, 600), "My window");
  auto err = glewInit();
  if (err != GLEW_OK) {
    std::string error("error initialing glew");
    throw std::runtime_error(error /*+ std::string(glewGetErrorString(err))*/);
  }
  
  glViewport(0, 0, 800, 600);

  std::cout << "GL Info: " << glGetString(GL_VERSION) << std::endl;

  time last_update_time = read_absolute_time();

  while (true) {
    {
      boost::unique_lock<boost::mutex> lock(mt);
      if (!update_thread_waiting) {
        cv.wait_for(lock, wait_time, [this] {
          return (bool) update_thread_waiting;
        });
      }

      update_drawers_if_nec();

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

    for(auto m_it = meshes.begin(); m_it != meshes.end();) {
      if (m_it->unique()) {
        m_it = meshes.erase(m_it);
      } else {
        auto &mesh = **m_it;

        if (mesh.absolute_last_update_time.is_zero()) {
          mesh.absolute_last_update_time = last_update_time;
        }
        mesh.draw(last_update_time - mesh.absolute_last_update_time);
        ++m_it;
      }
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
    for (auto& components_of_a_type : reg_components) {
      auto& vector_of_components = components_of_a_type.second;
      for (auto comp : vector_of_components) {
        auto drawer = dynamic_cast<render_component*>(comp)->create_drawer();
        auto d_meshes_its = drawer->get_meshes();
        for (auto it = d_meshes_its.first; it != d_meshes_its.second; ++it) {
          auto lower_bound = meshes.lower_bound(*it);
          if (*lower_bound != *it) {
            meshes.emplace_hint(lower_bound, *it);
          }
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
