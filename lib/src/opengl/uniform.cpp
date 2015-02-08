#include <algorithm>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/global_fun.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/sequenced_index.hpp>
#define INCLUDED_BY_UNIFORM_CPP
#include "opengl/uniform.h"
#include <unordered_map>
#include <memory>
#include <vector>
#include <string>

namespace game_engine {
namespace opengl {
static std::vector<int> prepare_vector(GLsizei count, unsigned components, const bool *data) {
  std::vector<int> ret;
  ret.reserve(count * components);
  std::copy(data, data + (count * components), std::back_inserter(ret));
  return ret;
}

void gl_uniform1bv(GLint location, GLsizei count, const bool *data) {
  auto vec(prepare_vector(count, 1, data));
  glUniform1iv(location, count, vec.data());
}

void gl_uniform2bv(GLint location, GLsizei count, const bool *data) {
  auto vec(prepare_vector(count, 1, data));
  glUniform2iv(location, count, vec.data());
}

void gl_uniform3bv(GLint location, GLsizei count, const bool *data) {
  auto vec(prepare_vector(count, 1, data));
  glUniform3iv(location, count, vec.data());
}

void gl_uniform4bv(GLint location, GLsizei count, const bool *data) {
  auto vec(prepare_vector(count, 1, data));
  glUniform4iv(location, count, vec.data());
}

struct names_struct {
 const char *cpp_name;
 const char *enum_name;
};

auto &get_names_map() {
  static std::unordered_map<GLenum, names_struct> names_map = [] {
    assert (names_array_counter < 100);
    std::unordered_map<GLenum, names_struct> ret;
    for (size_t i(0); i < names_array_counter; ++i) {
      ret.emplace(names_array[i].enum_val,
                       names_struct{names_array[i].cpp_name,
                                    names_array[i].enum_name});
    }
    return ret;
  }();
  return names_map;
}
const char *get_gl_type_string(GLenum enum_val) {
  return get_names_map().at(enum_val).enum_name;
}
const char *get_cpp_type_string(GLenum enum_val) {
  return get_names_map().at(enum_val).cpp_name;
}

uniform_setter::uniform_setter(uniform u_, GLint loc_, GLuint prog_id_)
 :uniform(std::move(u_)),
  location(loc_),
  prog_id(prog_id_)
{}

uniform_setter::uniform_setter()
{}

bool operator==(const uniform &lhs, const uniform &rhs) {
#define COMP(x) (lhs.x == rhs.x)
  return COMP(name) &&
         COMP(block_name) &&
         COMP(block_index) &&
         COMP(block_size) &&
         COMP(block_offset) &&
         COMP(array_size) &&
         COMP(block_array_stride) &&
         COMP(block_matrix_stride) &&
         COMP(matrix_row_major) &&
         COMP(type);
#undef COMP
}

bool operator!=(const uniform &lhs, const uniform &rhs) {
  return !(lhs == rhs);
}

std::vector<uniform_setter> get_uniforms_of_program(GLuint prog_id) {
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
  gl_vector locations(number_uniforms);
  std::vector<std::string> names(number_uniforms);

  GLint max_name_length;
  glGetProgramiv(prog_id, GL_ACTIVE_UNIFORM_MAX_LENGTH, &max_name_length);
  std::vector<char> name_buffer(max_name_length);

  for (size_t i = 0; i < (size_t)number_uniforms; ++i) {
    GLsizei real_length;
    glGetActiveUniformName(prog_id, i, max_name_length, &real_length, name_buffer.data());
    assert(real_length <= (GLsizei)max_name_length);
    names[i].assign(name_buffer.data());
    locations[i] = glGetUniformLocation(prog_id, names[i].c_str());
  }

  GLint number_blocks;
  gl_vector block_sizes;
  gl_vector name_lengths;
  std::vector<std::string> block_names;
  glGetProgramiv(prog_id, GL_ACTIVE_UNIFORM_BLOCKS, &number_blocks);

  if (number_blocks != 0) {
    auto get_block_data = [prog_id, number_blocks] (GLenum param)
      {
        std::vector<GLint> data_retrieved(number_blocks);
        for (size_t i = 0; i < (size_t)number_blocks; ++i) {
          glGetActiveUniformBlockiv(prog_id, i, param, &data_retrieved[i]);
        }
        return data_retrieved;
      };

    block_sizes = get_block_data(GL_UNIFORM_BLOCK_DATA_SIZE);
    name_lengths = get_block_data(GL_UNIFORM_BLOCK_NAME_LENGTH);
    block_names = std::vector<std::string>(number_blocks);

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
  }

  std::vector<uniform_setter> result(number_uniforms);
  for (size_t i = 0; i < (size_t)number_uniforms; ++i) {
    auto b_ind = block_indices[i];
    result[i] = {{names[i],
                  b_ind >= 0 ? block_names[b_ind] : "",
                  b_ind,
                  b_ind >= 0 ? block_sizes[b_ind] : -1,
                  offsets[i],
                  sizes[i],
                  array_strides[i],
                  matrix_strides[i],
                  row_major[i] == GL_TRUE,
                  (GLenum)types[i]},
                 locations[i],
                 prog_id};
  }

  return result;
}

uniform_block_binding::uniform_block_binding(GLint id_, std::string name_):
  id(id_),
  name(std::move(name_))
{}

const std::string &uniform_block_binding::get_name() const {
  return name;
}

GLint uniform_block_binding::get_id() const {
  return id;
}

std::shared_ptr<uniform_buffer_data> uniform_block_binding::get_bound_buffer_data() const {
  return bound_buffer_data.lock();
}

const std::string &get_name(const std::shared_ptr<uniform_block_binding> &ptr) {
  return ptr->get_name();
}

/*
 * The uniform block binding manager uses shared_ptrs to keep track of the bindings it
 * allocates for users. When a binding is no longer being used, its use_count() drops to 1,
 * which tells us that we are free to reuse it if we need to. If a binding the name passed as
 * the parameter has already been allocated, then that binding is returned again and, therefore,
 * its use_count() is raised to n+1, where n was its previous use_count().
 */

using binding_container = boost::multi_index_container<
                            std::shared_ptr<uniform_block_binding>,
                            boost::multi_index::indexed_by<
                              boost::multi_index::sequenced<>,
                              boost::multi_index::ordered_non_unique<
                                boost::multi_index::global_fun<
                                  const std::shared_ptr<uniform_block_binding>&,
                                  const std::string&,
                                  &get_name
                                >
                              >
                            >
                          >;

uniform_block_binding_handle get_free_uniform_block_binding(std::string name) {
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
