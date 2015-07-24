#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "opengl/uniform.h"
#include <sstream>
#include <utility>
#include "util/iter_to_ptr.h"

namespace game_engine {
namespace opengl {
void gl_uniform1bv(GLint location, GLsizei count, const bool* data);
void gl_uniform2bv(GLint location, GLsizei count, const bool* data);
void gl_uniform3bv(GLint location, GLsizei count, const bool* data);
void gl_uniform4bv(GLint location, GLsizei count, const bool* data);

template <typename T>
GLenum gl_type();

template <typename T>
const typename T::value_type *to_data_ptr(T* ptr, typename T::value_type = 0) {
  return glm::value_ptr(*ptr);
}

template <typename T>
T to_data_ptr(T ptr) {
  return ptr;
}

template <typename T, typename F>
auto call(F f, GLint loc, GLsizei count, T* ptr) -> decltype(f(loc, count, ptr)) {
  f(loc, count, ptr);
}

template <typename T, typename F>
auto call(F f, GLint loc, GLsizei count, T* ptr, bool = false) -> decltype(f(loc, count, GL_FALSE, ptr)) {
  f(loc, count, GL_FALSE, ptr);
}

#ifdef INCLUDED_BY_UNIFORM_CPP
struct gl_cpp_name_pair {
  GLenum enum_val;
  const char * cpp_name;
  const char * enum_name;
};
gl_cpp_name_pair names_array[100];
size_t names_array_counter(0);
#define GET_GL_TYPE(gl_t, cpp_t, set_f_t) \
template <> \
constexpr GLenum gl_type<cpp_t>() { return  gl_t; } \
void gl_uni_set_f(GLint loc, GLsizei count, const cpp_t *ptr) { \
  call(set_f_t, loc, count, to_data_ptr(ptr)); \
} \
bool initialize_names_map##gl_t = []{ names_array[names_array_counter++] \
                                        = {gl_t, #cpp_t, #gl_t}; \
                                      return true; }();
#else
#define GET_GL_TYPE(gl_t, cpp_t, set_f_t) \
template <> \
constexpr GLenum gl_type<cpp_t>() { return  gl_t; } \
void gl_uni_set_f(GLint loc, GLsizei count, const cpp_t *ptr);
#endif
#undef INCLUDED_BY_UNIFORM_CPP

GET_GL_TYPE(GL_FLOAT, float, glUniform1fv)
GET_GL_TYPE(GL_FLOAT_VEC2, glm::vec2, glUniform2fv)
GET_GL_TYPE(GL_FLOAT_VEC3, glm::vec3, glUniform3fv)
GET_GL_TYPE(GL_FLOAT_VEC4, glm::vec4, glUniform4fv)
GET_GL_TYPE(GL_DOUBLE, double, glUniform1dv)
GET_GL_TYPE(GL_DOUBLE_VEC2, glm::dvec2, glUniform2dv)
GET_GL_TYPE(GL_DOUBLE_VEC3, glm::dvec3, glUniform3dv)
GET_GL_TYPE(GL_DOUBLE_VEC4, glm::dvec4, glUniform4dv)
GET_GL_TYPE(GL_INT, int, glUniform1iv)
GET_GL_TYPE(GL_INT_VEC2, glm::ivec2, glUniform2iv)
GET_GL_TYPE(GL_INT_VEC3, glm::ivec3, glUniform3iv)
GET_GL_TYPE(GL_INT_VEC4, glm::ivec4, glUniform4iv)
GET_GL_TYPE(GL_UNSIGNED_INT, unsigned int, glUniform1uiv)
GET_GL_TYPE(GL_UNSIGNED_INT_VEC2, glm::uvec2, glUniform2uiv)
GET_GL_TYPE(GL_UNSIGNED_INT_VEC3, glm::uvec3, glUniform3uiv)
GET_GL_TYPE(GL_UNSIGNED_INT_VEC4, glm::uvec4, glUniform4uiv)
GET_GL_TYPE(GL_BOOL, bool, gl_uniform1bv)
GET_GL_TYPE(GL_BOOL_VEC2, glm::bvec2, gl_uniform2bv)
GET_GL_TYPE(GL_BOOL_VEC3, glm::bvec3, gl_uniform3bv)
GET_GL_TYPE(GL_BOOL_VEC4, glm::bvec4, gl_uniform4bv)
GET_GL_TYPE(GL_FLOAT_MAT2, glm::mat2, glUniformMatrix2fv)
GET_GL_TYPE(GL_FLOAT_MAT3, glm::mat3, glUniformMatrix3fv)
GET_GL_TYPE(GL_FLOAT_MAT4, glm::mat4, glUniformMatrix4fv)
GET_GL_TYPE(GL_FLOAT_MAT2x3, glm::mat2x3, glUniformMatrix2x3fv)
GET_GL_TYPE(GL_FLOAT_MAT2x4, glm::mat2x4, glUniformMatrix2x4fv)
GET_GL_TYPE(GL_FLOAT_MAT3x2, glm::mat3x2, glUniformMatrix3x2fv)
GET_GL_TYPE(GL_FLOAT_MAT3x4, glm::mat3x4, glUniformMatrix3x4fv)
GET_GL_TYPE(GL_FLOAT_MAT4x2, glm::mat4x2, glUniformMatrix4x2fv)
GET_GL_TYPE(GL_FLOAT_MAT4x3, glm::mat4x3, glUniformMatrix4x3fv)
GET_GL_TYPE(GL_DOUBLE_MAT2, glm::dmat2, glUniformMatrix2dv)
GET_GL_TYPE(GL_DOUBLE_MAT3, glm::dmat3, glUniformMatrix3dv)
GET_GL_TYPE(GL_DOUBLE_MAT4, glm::dmat4, glUniformMatrix4dv)
GET_GL_TYPE(GL_DOUBLE_MAT2x3, glm::dmat2x3, glUniformMatrix2x3dv)
GET_GL_TYPE(GL_DOUBLE_MAT2x4, glm::dmat2x4, glUniformMatrix2x4dv)
GET_GL_TYPE(GL_DOUBLE_MAT3x2, glm::dmat3x2, glUniformMatrix3x2dv)
GET_GL_TYPE(GL_DOUBLE_MAT3x4, glm::dmat3x4, glUniformMatrix3x4dv)
GET_GL_TYPE(GL_DOUBLE_MAT4x2, glm::dmat4x2, glUniformMatrix4x2dv)
GET_GL_TYPE(GL_DOUBLE_MAT4x3, glm::dmat4x3, glUniformMatrix4x3dv)
/* TODO: samplers
GET_GL_TYPE(GL_SAMPLER_1D, sampler1D)
GET_GL_TYPE(GL_SAMPLER_2D, sampler2D)
GET_GL_TYPE(GL_SAMPLER_3D, sampler3D)
GET_GL_TYPE(GL_SAMPLER_CUBE, samplerCube)
GET_GL_TYPE(GL_SAMPLER_1D_SHADOW, sampler1DShadow)
GET_GL_TYPE(GL_SAMPLER_2D_SHADOW, sampler2DShadow)
GET_GL_TYPE(GL_SAMPLER_1D_ARRAY, sampler1DArray)
GET_GL_TYPE(GL_SAMPLER_2D_ARRAY, sampler2DArray)
GET_GL_TYPE(GL_SAMPLER_1D_ARRAY_SHADOW, sampler1DArrayShadow)
GET_GL_TYPE(GL_SAMPLER_2D_ARRAY_SHADOW, sampler2DArrayShadow)
GET_GL_TYPE(GL_SAMPLER_2D_MULTISAMPLE, sampler2DMS)
GET_GL_TYPE(GL_SAMPLER_2D_MULTISAMPLE_ARRAY, sampler2DMSArray)
GET_GL_TYPE(GL_SAMPLER_CUBE_SHADOW, samplerCubeShadow)
GET_GL_TYPE(GL_SAMPLER_BUFFER, samplerBuffer)
GET_GL_TYPE(GL_SAMPLER_2D_RECT, sampler2DRect)
GET_GL_TYPE(GL_SAMPLER_2D_RECT_SHADOW, sampler2DRectShadow)
GET_GL_TYPE(GL_INT_SAMPLER_1D, isampler1D)
GET_GL_TYPE(GL_INT_SAMPLER_2D, isampler2D)
GET_GL_TYPE(GL_INT_SAMPLER_3D, isampler3D)
GET_GL_TYPE(GL_INT_SAMPLER_CUBE, isamplerCube)
GET_GL_TYPE(GL_INT_SAMPLER_1D_ARRAY, isampler1DArray)
GET_GL_TYPE(GL_INT_SAMPLER_2D_ARRAY, isampler2DArray)
GET_GL_TYPE(GL_INT_SAMPLER_2D_MULTISAMPLE, isampler2DMS)
GET_GL_TYPE(GL_INT_SAMPLER_2D_MULTISAMPLE_ARRAY, isampler2DMSArray)
GET_GL_TYPE(GL_INT_SAMPLER_BUFFER, isamplerBuffer)
GET_GL_TYPE(GL_INT_SAMPLER_2D_RECT, isampler2DRect)
GET_GL_TYPE(GL_UNSIGNED_INT_SAMPLER_1D, usampler1D)
GET_GL_TYPE(GL_UNSIGNED_INT_SAMPLER_2D, usampler2D)
GET_GL_TYPE(GL_UNSIGNED_INT_SAMPLER_3D, usampler3D)
GET_GL_TYPE(GL_UNSIGNED_INT_SAMPLER_CUBE, usamplerCube)
GET_GL_TYPE(GL_UNSIGNED_INT_SAMPLER_1D_ARRAY, usampler2DArray)
GET_GL_TYPE(GL_UNSIGNED_INT_SAMPLER_2D_ARRAY, usampler2DArray)
GET_GL_TYPE(GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE, usampler2DMS)
GET_GL_TYPE(GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY, usampler2DMSArray)
GET_GL_TYPE(GL_UNSIGNED_INT_SAMPLER_BUFFER, usamplerBuffer)
GET_GL_TYPE(GL_UNSIGNED_INT_SAMPLER_2D_RECT, usampler2DRec)
*/

#undef GET_GL_TYPE

const char * get_gl_type_string(GLenum);
const char * get_cpp_type_string(GLenum);

// documented in program.h
void bind_program(GLuint prog_id);

template <typename T>
void check_cpp_gl_compatibility(GLenum uniform_gl_t) {
  auto gl_t = gl_type<T>();
  if (uniform_gl_t != gl_t) {
    std::stringstream ss;
    ss << "tried to assign a " << get_cpp_type_string(gl_t)
       << " (OpenGL type: " << get_gl_type_string(gl_t)
       << ") to a uniform of type " << get_cpp_type_string(uniform_gl_t)
       << " (OpenGL type: " << get_gl_type_string(uniform_gl_t) << ")";
    throw uniform_setter::type_mismatch(ss.str());
  }
}

template <typename T>
void uniform_setter::set(const T &t) {
  set(1, &t);
}

template <typename It>
void uniform_setter::set(GLsizei count, It begin) {
  check_cpp_gl_compatibility<typename std::iterator_traits<It>::value_type>(type);

  bind_program(prog_id);
  auto end(begin);
  std::advance(end, count);
  auto data_ptr = util::trans_iter_to_ptr(begin, end);
  gl_uni_set_f(location, count, data_ptr.begin);
}

template <typename It>
void uniform_setter::set(It begin, It end) {
  set(std::distance(begin, end), std::move(begin));
}
}
}
