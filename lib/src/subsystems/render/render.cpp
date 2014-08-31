#include <GL/glew.h>
#include <SFML/Window.hpp>
#include "subsystems/render/render.h"

namespace game_engine {
namespace logic {
namespace subsystems {
namespace render {
render_subsystem::render_subsystem()
{}

render_subsystem::~render_subsystem()
{}

void render_subsystem::handle_events() {
  sf::Window window(sf::VideoMode(800, 600), "My window");
  glewInit();

  while (window.isOpen()) {
    sf::Event event;
    while (window.pollEvent(event)) {
      if (event.type == sf::Event::Closed) {
        window.close();
				return;
      }
    }
  }
}

void render_subsystem::update_all()
{}
}
}
}
}
