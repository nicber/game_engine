#include <GL/glew.h>
#include <SFML/Window.hpp>
#include "subsystems/render/render.h"

namespace game_engine {
namespace logic {
namespace subsystems {
namespace render {
render_subsystem::render_subsystem(std::atomic<bool> &exit_flag)
 : window_thr([this, &exit_flag] { window_loop(exit_flag); })
{}

render_subsystem::~render_subsystem() {
  window_thr.join();
}

void render_subsystem::window_loop(std::atomic<bool> &exit_flag) {
  sf::Window window(sf::VideoMode(800, 600), "My window");
  glewInit();

  while (window.isOpen()) {
    sf::Event event;
    while (window.pollEvent(event)) {
      if (event.type == sf::Event::Closed) {
        exit_flag = true;
        window.close();
      }
    }
  }
}
}
}
}
}
