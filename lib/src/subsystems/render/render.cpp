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
  sf::Window window(sf::VideoMode(800, 600), "My window");
  glewInit();
  
  time last_update_time = read_absolute_time();
  unsigned long long time_since_last_update = 0;
  std::vector<drawer> drawers;
  
  while (window.isOpen()) {
    bool data_just_uploaded = false;
    
    if (update_thread_waiting) {
      drawers.clear();
      for (auto& components_of_a_type : reg_components) {
        auto& vector_of_components = components_of_a_type.second;
        for (auto comp : vector_of_components) {
          auto drawer = static_cast<render_component*>(comp)->create_drawer();
          if (!drawers.size() || !drawers.back().try_combine_with(drawer)) {
            drawers.emplace_back(std::move(drawer));
          }
        }
      }
      
      data_just_uploaded = true;
      update_thread_waiting = false;
      boost::lock_guard<boost::mutex> lock(mt);
      cv.notify_one();
    }
    
    if (data_just_uploaded) {
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
      return;
    }
    
    for (auto& drawer : drawers) {
      drawer.draw(time_since_last_update);
    }
  }
}

void render_subsystem::update_all() {
  assert(!update_thread_waiting);
  boost::unique_lock<boost::mutex> lock(mt);
  update_thread_waiting = true;
  cv.wait(lock, [&] { return !update_thread_waiting; });
}
}
}
}
}
