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

class triangle_drawer : public drawer {
public:
  triangle_drawer(triangle_renderer &tri_renderer):
    triang_renderer(&tri_renderer)
  {}

  void draw(unsigned long long delta) final override;

  bool try_combine_with(drawer &other) final override {
    return false;
  }

private:
  triangle_renderer *triang_renderer = nullptr;
};

class triangle_renderer : public render_component
{
public:
  triangle_renderer() {}

  std::unique_ptr<drawer> create_drawer() final override {
    return std::unique_ptr<drawer>(new triangle_drawer(*this));
  }

private:
  struct render_data {
    static program create_program() {
      std::string vertex_source =
      "in vec2 position;"
      "void main() {"
      "  gl_Position = vec4(position, 0, 1);"
      "}";

      std::string fragment_source =
      "void main() { gl_FragColor = vec4(1,1,1,0); }";

      shader vertex_shader(shader_type::vertex, vertex_source, {});
      shader fragment_shader(shader_type::fragment, fragment_source, {});
      program res(vertex_shader, fragment_shader, {}, {attrib_pair{"position", 1}}, {});
      return res;
    }

    buffer<vec2> points = buffer<vec2>(3, buf_freq_access::mod_once,
                                      buf_kind_access::read_gl_write_app);
    program ogl_program = create_program();
    vertex_array_object vao;
    render_data() {
      std::vector<vec2> data = {{0.5,0},
                               {0,sqrt(3)/2},
                               {-0.5, 0}};

      std::copy(data.cbegin(), data.cend(), points.begin());

      vao.bind_buffer_to_attrib<vec2, float, 2>(points, 0, 0,
                                               ogl_program.get_vertex_attr("position"));
    }
  };

  friend class triangle_drawer;
  std::unique_ptr<render_data> data = nullptr;
};

void triangle_drawer::draw(unsigned long long) {
  if (!triang_renderer->data) {
    triang_renderer->data.reset(new triangle_renderer::render_data);
  }
  triang_renderer->data->vao.bind();
  triang_renderer->data->ogl_program.bind();
  glDrawArrays(GL_TRIANGLES, 0, 3);
}
void GAME_EXECUTION_FUNCTION (game_engine::logic::game *gam) {
  render_subsystem *rend_sub = nullptr;

  try {
    rend_sub = &gam->get<render_subsystem>();
  } catch (game::no_subsystem_found<render_subsystem> &) {
    std::cerr << "No rendering subsystem found.\n"
      "WARNING: There's no way to know that the game should stop,\n so it will"
      "         run indefinitely.";
  }

  std::unique_ptr<entity> temp = std::unique_ptr<entity>(new entity);
  auto &entity = *temp;
  entity.new_component<triangle_renderer>();
  gam->add_entity(std::move(temp));

  while(!rend_sub || !rend_sub->has_exited()) {
    gam->tick();
  }
}
