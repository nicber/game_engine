#include "render/mesh.h"

namespace game_engine {
namespace render {
mesh::mesh(std::shared_ptr<opengl::program> prog_,
           std::shared_ptr<opengl::vertex_array_object> vao_,
           size_t count_,
           size_t base_index_,
           size_t base_vertex_,
           size_t instance_count_,
           size_t base_instance_)
 :prog(std::move(prog_)),
  vao(std::move(vao_)),
  count(count_),
  base_index(base_index_),
  base_vertex(base_vertex_),
  instance_count(instance_count_),
  base_instance(base_instance_)
{}

bool mesh::operator<(const mesh &rhs) const {
  if (vao->vao_id < rhs.vao->vao_id) {
    return true;
  } else if (vao->vao_id > rhs.vao->vao_id) {
    return false;
  } else { // vao->vao_id == rhs.vao->vao_id
    return prog->program_id < rhs.prog->program_id;
  }
}

void mesh::draw(unsigned long long delta_time) const {
  vao->bind();
  prog->bind();

  auto index = reinterpret_cast<const void*>(base_index);
  glDrawElementsInstancedBaseVertexBaseInstance(GL_TRIANGLES, // mode
                                                count, // number of prims
                                                GL_UNSIGNED_SHORT, // type of indices
                                                index, // offset in indices array
                                                instance_count,
                                                base_vertex,
                                                base_instance);
}
}
}
