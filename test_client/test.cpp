#include <iostream>
#include "launcher_api.h"
#include "opengl/buffer.h"
#include "opengl/program.h"
#include "opengl/vertex_array_object.h"
#include "subsystems/render/render.h"
#include "world.h"

using namespace game_engine::logic;
using namespace game_engine::opengl;
using namespace game_engine::logic::subsystems::render;

struct vec2 {
  float x, y;
};

class triangle_renderer;

class triangle_renderer : public render_component
{
public:
  triangle_renderer() {}

  std::shared_ptr<const drawer> create_drawer() final override {
    if (!drawer_ptr) {
      construct_drawer();
    }
    return drawer_ptr;
  }

private:
    void construct_drawer() {
      auto points = std::make_shared<buffer<vec2>>(3, buf_freq_access::mod_once,
                                                   buf_kind_access::read_gl_write_app);

      auto indices = std::make_shared<buffer<unsigned short>>(3, buf_freq_access::mod_once,
                                                              buf_kind_access::read_gl_write_app);

      vec2 data[] = {{0.5,0}, {0,sqrt(3)/2}, {-0.5, 0}};
      std::copy(std::begin(data), std::end(data), points->begin());

      unsigned short ind_data[] = {0, 1, 2};
      std::copy(std::begin(ind_data), std::end(ind_data), indices->begin());

      std::string vertex_source =
        "in vec2 position;"
        "void main() {"
        "  gl_Position = vec4(position, 0, 1);"
        "}";

      std::string fragment_source =
        "void main() { gl_FragColor = vec4(1,1,1,0); }";

      shader vertex_shader(shader_type::vertex, vertex_source, {});
      shader fragment_shader(shader_type::fragment, fragment_source, {});
      auto ogl_program = std::make_shared<program>(program(vertex_shader, fragment_shader, {}, {attrib_pair{"position", 1}}, {}));

      auto vao = std::make_shared<vertex_array_object>();
      vao->bind_buffer_to_attrib<vec2, float, 2>(std::move(points), 0, 0,
                                                ogl_program->get_vertex_attr("position"));
      vao->bind_buffer_to_elements_array(std::move(indices));

      auto mesh_ptr = std::make_shared<game_engine::render::mesh>(std::move(ogl_program),
                                                             std::move(vao),
                                                             3,
                                                             0,
                                                             0,
                                                             1,
                                                             0);

      drawer_ptr = std::make_shared<drawer>();
      *drawer_ptr += mesh_ptr;
    }
  std::shared_ptr<drawer> drawer_ptr;
};

extern "C" {
void start_game(game_engine::logic::game *gam) {
  render_subsystem *rend_sub = nullptr;

  try {
    rend_sub = &gam->get<render_subsystem>();
  } catch (game::no_subsystem_found<render_subsystem> &) {
    std::cerr << "No rendering subsystem found.\n"
      "WARNING: There's no way to know that the game should stop,\n"
      "         so it will run indefinitely.";
  }

  std::unique_ptr<entity> temp = std::unique_ptr<entity>(new entity);
  auto &entity = *temp;
  entity.new_component<triangle_renderer>();
  gam->add_entity(std::move(temp));

  while(!rend_sub || !rend_sub->has_exited()) {
    gam->tick();
  }
}
}
