#include <algorithm>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/global_fun.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/sequenced_index.hpp>
#include "opengl/uniform.h"
#include <memory>
#include <vector>
#include <string>

namespace game_engine {
namespace opengl {
std::vector<uniform> get_uniforms_of_program(GLuint prog_id) {
  GLint number_uniforms;
  glGetProgramiv(prog_id, GL_ACTIVE_UNIFORMS, &number_uniforms);

  std::vector<GLuint> uniform_indices;
  uniform_indices.reserve(number_uniforms);
  for (size_t i = 0; i < (size_t)number_uniforms; ++i) {
    uniform_indices.emplace_back(i);
  }

  auto get_data = [prog_id, &uniform_indices, number_uniforms] (GLenum param)
    {
      std::vector<GLint> data_retrieved(number_uniforms);
      glGetActiveUniformsiv(prog_id, number_uniforms, uniform_indices.data(),
                            param, data_retrieved.data());
      return data_retrieved;
    };

  using gl_vector = std::vector<GLint>;
  gl_vector types = get_data(GL_UNIFORM_TYPE);
  gl_vector sizes = get_data(GL_UNIFORM_SIZE);
  gl_vector block_indices = get_data(GL_UNIFORM_BLOCK_INDEX);
  gl_vector offsets = get_data(GL_UNIFORM_OFFSET);
  gl_vector array_strides = get_data(GL_UNIFORM_ARRAY_STRIDE);
  gl_vector matrix_strides = get_data(GL_UNIFORM_MATRIX_STRIDE);
  gl_vector row_major = get_data(GL_UNIFORM_IS_ROW_MAJOR);
  std::vector<std::string> names(number_uniforms);

  GLint max_name_length;
  glGetProgramiv(prog_id, GL_ACTIVE_UNIFORM_MAX_LENGTH, &max_name_length);
  std::vector<char> name_buffer(max_name_length);

  for (size_t i = 0; i < (size_t)number_uniforms; ++i) {
    GLsizei real_length;
    glGetActiveUniformName(prog_id, i, max_name_length, &real_length, name_buffer.data());
    assert(real_length <= (GLsizei)max_name_length);
    names[i].assign(name_buffer.data());
  }

  GLint number_blocks;
  glGetProgramiv(prog_id, GL_ACTIVE_UNIFORM_BLOCKS, &number_blocks);

  auto get_block_data = [prog_id, number_blocks] (GLenum param)
    {
      std::vector<GLint> data_retrieved(number_blocks);
      for (size_t i = 0; i < (size_t)number_blocks; ++i) {
        glGetActiveUniformBlockiv(prog_id, i, param, &data_retrieved[i]);
      }
      return data_retrieved;
    };

  gl_vector block_sizes = get_block_data(GL_UNIFORM_BLOCK_DATA_SIZE);
  gl_vector name_lengths = get_block_data(GL_UNIFORM_BLOCK_NAME_LENGTH);
  std::vector<std::string> block_names(number_blocks);
  size_t max_block_name_length = *std::max_element(name_lengths.begin(),
                                                   name_lengths.end());
  std::vector<char> block_name_buffer(max_block_name_length);

  for (size_t i = 0; i < (size_t)number_blocks; ++i) {
    GLsizei real_length;
    glGetActiveUniformBlockName(prog_id, i, max_block_name_length, &real_length,
                                block_name_buffer.data());
    assert(real_length <= (GLsizei)max_block_name_length);
    block_names[i].assign(block_name_buffer.data());
  }

  std::vector<uniform> result(number_uniforms);
  for (size_t i = 0; i < (size_t)number_uniforms; ++i) {
    auto b_ind = block_indices[i];
    result[i] = {names[i],
                 block_names[b_ind],
                 b_ind,
                 block_sizes[b_ind],
                 offsets[i],
                 sizes[i],
                 array_strides[i],
                 matrix_strides[i],
                 row_major[i] == GL_TRUE,
                 (GLenum)types[i]};
  }

  return result;
}

uniform_block_binding::uniform_block_binding(GLint id_, std::string name_):
  id(id_),
  name(std::move(name_))
{}

const std::string &get_name(const std::shared_ptr<uniform_block_binding> &ptr) {
  return ptr->name;
}

using binding_container = boost::multi_index_container<
                            std::shared_ptr<uniform_block_binding>,
                            boost::multi_index::indexed_by<
                              boost::multi_index::sequenced<>,
                              boost::multi_index::ordered_unique<
                                boost::multi_index::global_fun<
                                  const std::shared_ptr<uniform_block_binding>&,
                                  const std::string&,
                                  &get_name
                                >
                              >
                            >
                          >;

std::shared_ptr<uniform_block_binding> get_free_uniform_block_binding(std::string name) {
  static binding_container bindings = [] {
    GLint tmp;
    glGetIntegerv(GL_MAX_UNIFORM_BUFFER_BINDINGS, &tmp);
    binding_container res;
    for (size_t i = 0; i < (size_t) tmp; ++i) {
      res.push_back(std::make_shared<uniform_block_binding>(i, ""));
    }
    return res;
    } ();

  auto name_search_it = bindings.get<1>().find(name);
  if (name_search_it != bindings.get<1>().end()) {
    return *name_search_it;
  }

  for (auto it = bindings.begin(); it != bindings.end(); ++it) {
    if (it->use_count() == 1) {
      bindings.modify(it, [name = std::move(name)] (std::shared_ptr<uniform_block_binding> &ptr) {
        ptr->name = std::move(name);
      });
      return *it;
    }
  }

  throw std::runtime_error("no uniform block bindings available");
}
}
}
