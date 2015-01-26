#pragma once

#include <algorithm>
#include "opengl/buffer.h"
#include <stdexcept>

namespace game_engine {
namespace opengl {
template <typename T>
buffer<T>::buffer_accessor::buffer_accessor(const buffer<T> &buf, std::ptrdiff_t offs)
  : buff(&buf),
    offset(offs)
{}

template <typename T>
typename buffer<T>::buffer_accessor &buffer<T>::buffer_accessor::operator=(const T &data) {
  if (offset >= buff->buffer_size) {
    throw std::runtime_error("attempted to read past the end of buffer");
  }

  glBindBuffer(GL_COPY_WRITE_BUFFER, buff->buffer_id);
  glBufferSubData(GL_COPY_WRITE_BUFFER,
                  offset * sizeof(T),
                  sizeof(T),
                  &data);

  buff->change_signal();

  return *this;
}

template <typename T>
buffer<T>::buffer_accessor::operator T() const {
  if (offset >= buff->buffer_size) {
    throw std::runtime_error("attempted to read past the end of buffer");
  }

  glBindBuffer(GL_COPY_READ_BUFFER, buff->buffer_id);

  T temp;
  glGetBufferSubData(GL_COPY_WRITE_BUFFER,
                     offset * sizeof(T),
                     sizeof(T),
                     &temp);

  return temp;
}

template <typename T>
buffer<T>::buffer(size_t size, buf_freq_access freq_acc, buf_kind_access kind_acc)
  : buffer_size(size)
{
  GLenum usage;
  switch (freq_acc) {
    case buf_freq_access::mod_once:
      switch (kind_acc) {
        case buf_kind_access::read_gl_write_app:
          usage = GL_STATIC_DRAW;
          break;
        case buf_kind_access::read_app_write_gl:
          usage = GL_STATIC_READ;
          break;
        case buf_kind_access::read_gl_write_gl:
          usage = GL_STATIC_COPY;
          break;
      }
      break;

    case buf_freq_access::mod_freq:
      switch (kind_acc) {
        case buf_kind_access::read_gl_write_app:
          usage = GL_STREAM_DRAW;
          break;
        case buf_kind_access::read_app_write_gl:
          usage = GL_STREAM_READ;
          break;
        case buf_kind_access::read_gl_write_gl:
          usage = GL_STREAM_COPY;
          break;
      }
      break;

    case buf_freq_access::mod_little:
      switch (kind_acc) {
        case buf_kind_access::read_gl_write_app:
          usage = GL_DYNAMIC_DRAW;
          break;
        case buf_kind_access::read_app_write_gl:
          usage = GL_DYNAMIC_READ;
          break;
        case buf_kind_access::read_gl_write_gl:
          usage = GL_DYNAMIC_COPY;
          break;
      }
      break;
  }

  create_buffer(size, usage);
}

template <typename T>
buffer<T>::buffer(buffer<T> &&other) {
  using std::swap;
  buffer<T> tmp;
  swap(*this, tmp);
  swap(*this, other);
}

template <typename T>
buffer<T> &buffer<T>::operator=(buffer<T> &&rhs) {
  using std::swap;
  buffer<T> tmp;
  swap(*this, tmp);
  swap(*this, rhs);
}

template <typename T>
buffer<T>::~buffer() {
  glDeleteBuffers(1, &buffer_id);
}

template <typename T>
const typename buffer<T>::buffer_accessor buffer<T>::operator[](std::ptrdiff_t index) const {
  return {this, index};
}

template <typename T>
typename buffer<T>::buffer_accessor buffer<T>::operator[](std::ptrdiff_t index) {
  return {this, index};
}

template <typename T>
typename buffer<T>::buffer_iterator buffer<T>::begin() {
  return {*this, 0};
}

template <typename T>
typename buffer<T>::const_buffer_iterator buffer<T>::begin() const {
  return {*this, 0};
}

template <typename T>
typename buffer<T>::const_buffer_iterator buffer<T>::cbegin() const {
  return {*this, 0};
}

template <typename T>
typename buffer<T>::buffer_iterator buffer<T>::end() {
  return {*this, buffer_size};
}

template <typename T>
typename buffer<T>::const_buffer_iterator buffer<T>::end() const {
  return {*this, buffer_size};
}


template <typename T>
boost::signals2::scoped_connection buffer<T>::on_change_connect(on_change_slot slot) const {
  return change_signal.connect(std::move(slot));
}

template <typename T>
void buffer<T>::resize(std::ptrdiff_t to) {
  if (to == buffer_size) {
    return;
  }

  std::ptrdiff_t max_to_copy = std::min(to, buffer_size);

  buffer<T> temp_buffer(to, creation_flags);

  glBindBuffer(GL_COPY_WRITE_BUFFER, temp_buffer.buffer_id);
  glBindBuffer(GL_COPY_READ_BUFFER, buffer_id);

  glCopyBufferSubData(GL_COPY_READ_BUFFER,
                      GL_COPY_WRITE_BUFFER,
                      0,
                      0,
                      max_to_copy * sizeof(T));

  using std::swap;
  swap(*this, temp_buffer);
}

template <typename T>
std::ptrdiff_t buffer<T>::size() const {
  return buffer_size;
}

template <typename T>
GLuint buffer<T>::get_buffer_id() const {
  return buffer_id;
}

template <typename T>
buffer<T>::buffer() {
}

template <typename T>
buffer<T>::buffer(size_t size, GLenum flags) {
  create_buffer(size, flags);
}

template <typename T>
void buffer<T>::create_buffer(size_t size, GLenum flags) {
  buffer_size = size;
  creation_flags = flags;

  glGenBuffers(1, &buffer_id);

  glBindBuffer(GL_COPY_WRITE_BUFFER, buffer_id);

  glBufferData(GL_COPY_WRITE_BUFFER,
               buffer_size * sizeof(T),
               nullptr,
               creation_flags);
}

template <typename T>
buffer<T>::buffer_iterator_base::buffer_iterator_base(const buffer& buf,
                                                      std::ptrdiff_t offs)
  : buff_acc(buf, offs)
{}

template <typename T>
typename buffer<T>::buffer_accessor &buffer<T>::buffer_iterator_base::dereference() const {
  return buff_acc;
}

template <typename T>
bool buffer<T>::buffer_iterator_base::equal(const buffer<T>::buffer_iterator_base &other) const {
  return buff_acc.offset == other.buff_acc.offset && buff_acc.buff == other.buff_acc.buff;
}

template <typename T>
void buffer<T>::buffer_iterator_base::increment() {
  ++buff_acc.offset;
}

template <typename T>
void buffer<T>::buffer_iterator_base::decrement() {
  --buff_acc.offset;
}

template <typename T>
void buffer<T>::buffer_iterator_base::advance(std::ptrdiff_t n) {
  buff_acc.offset += n;
}

template <typename T>
std::ptrdiff_t buffer<T>::buffer_iterator_base::distance_to(const buffer_iterator_base &other) const {
  return other.buff_acc.offset - other.buff_acc.offset;
}

template <typename T>
void swap(buffer<T> &lhs, buffer<T> &rhs) {
  using std::swap;
  swap(lhs.change_signal, rhs.change_signal);
  swap(lhs.buffer_id, rhs.buffer_id);
  swap(lhs.buffer_size, rhs.buffer_size);
  swap(lhs.creation_flags, rhs.creation_flags);
}

}
}
