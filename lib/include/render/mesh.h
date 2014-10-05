#pragma once

#include <memory>
#include "opengl/program.h"
#include "opengl/vertex_array_object.h"
#include <vector>

namespace game_engine {
namespace render {
class mesh {
public:
  mesh(std::shared_ptr<opengl::program> prog_,
       std::shared_ptr<opengl::vertex_array_object> vao_,
       size_t count_,
       size_t base_index_,
       size_t base_vertex_,
       size_t instance_count_,
       size_t base_instance_);

  bool operator<(const mesh &rhs) const;

  void draw(unsigned long long delta_time) const;
private:
  std::shared_ptr<opengl::program> prog;
  std::shared_ptr<opengl::vertex_array_object> vao;
  size_t count;
  size_t base_index;
  size_t base_vertex;
  size_t instance_count;
  size_t base_instance;
};
}
}
