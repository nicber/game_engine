#pragma once

#include <memory>
#include "opengl/program.h"
#include "opengl/uniform_buffer.h"
#include "opengl/vertex_array_object.h"
#include <vector>

namespace game_engine {
namespace render {
class mesh {
public:
  using uni_buff_vector = std::vector<std::shared_ptr<const opengl::uniform_buffer>>;
public:
  mesh(std::shared_ptr<const opengl::program> prog_,
       std::shared_ptr<const opengl::vertex_array_object> vao_,
       uni_buff_vector uni_buffs_,
       size_t count_,
       size_t base_index_,
       size_t base_vertex_,
       size_t instance_count_,
       size_t base_instance_);

  bool operator<(const mesh &rhs) const;

  void draw(unsigned long long delta_time) const;
private:
  std::shared_ptr<const opengl::program> prog;
  std::shared_ptr<const opengl::vertex_array_object> vao;
  uni_buff_vector uni_buffs;
  const size_t count;
  const size_t base_index;
  const size_t base_vertex;
  const size_t instance_count;
  const size_t base_instance;
};
}
}
