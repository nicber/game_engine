#pragma once
#include <boost/iterator/iterator_facade.hpp>
#include <GL/glew.h>
#include <type_traits>

namespace game_engine {
namespace opengl {
class program;

/** \brief Enum for specifying the frequency of modification of the buffer.
 * buf_freq_access::mod_once = set once and then used.
 * buf_freq_access::mod_little = set once and used several times, several times.
 * buf_freq_access::mod_freq = set once and used once, continously.
 */
enum class buf_freq_access {
  mod_once,
  mod_little,
  mod_freq
};

/** \brief Enum for specifying the kind of access of the buffer.
 * Self explanatory.
 */
enum class buf_kind_access {
  read_app_write_gl,
  read_gl_write_app,
  read_gl_write_gl
};

/** \brief Base class for buffers.
 */
class buffer_base {
};

/** \brief Template that represents a GL buffer that stores data of type T.
 */
template <typename T>
class buffer : public buffer_base {
  public:
    class buffer_accessor;

  private:
    class buffer_iterator_base {
    public:
      buffer_iterator_base(const buffer& buf, std::ptrdiff_t offs);

      buffer_accessor &dereference() const;

      bool equal(const buffer_iterator_base& other) const;

      void increment();

      void decrement();

      void advance(std::ptrdiff_t n);

      std::ptrdiff_t distance_to(const buffer_iterator_base &other) const;

      mutable buffer_accessor buff_acc; // kinda hacky.
    };

public:
  static_assert(std::has_trivial_copy_constructor<T>::value,
      "T can't have custom copy or move operations");

  /** \brief Proxy class that is returned from the dereference operation
   * of iterators. Abstracts uploading and downloading data from the GPU.
   */
  class buffer_accessor {
  public:
    buffer_accessor(const buffer<T>& buf, std::ptrdiff_t offs);

    buffer_accessor();

    buffer_accessor &operator=(const T &data);
    operator T() const;

  private:
    friend class buffer_iterator_base;

    const buffer * const buff = nullptr;
    std::ptrdiff_t offset;
  };

  class const_buffer_iterator:
    protected buffer_iterator_base,
    public boost::iterator_facade<const_buffer_iterator, const buffer_accessor, boost::random_access_traversal_tag>
  {
    friend boost::iterator_core_access;
  public:
    using buffer_iterator_base::buffer_iterator_base;
  };

  class buffer_iterator:
    protected buffer_iterator_base,
    public boost::iterator_facade<buffer_iterator, buffer_accessor, boost::random_access_traversal_tag>
  {
    friend boost::iterator_core_access;
  public:
    using buffer_iterator_base::buffer_iterator_base;
    buffer_iterator(const const_buffer_iterator &other);
  };

public:
  /** \brief Constructs a buffer of with size size, access frequency equal
   * to freq_acc, and type of access equal to kind_acc.
   */
  buffer(size_t size, buf_freq_access freq_acc, buf_kind_access kind_acc);

  buffer(buffer &&other);
  buffer &operator=(buffer &&other);

  ~buffer();

  const buffer_accessor operator[](std::ptrdiff_t index) const;
  buffer_accessor operator[](std::ptrdiff_t index);

  buffer_iterator begin();
  const_buffer_iterator begin() const;
  const_buffer_iterator cbegin() const;

  buffer_iterator end();
  const_buffer_iterator end() const;
  const_buffer_iterator cend() const;

  using iterator = buffer_iterator;
  using const_iterator = const_buffer_iterator;

  /** \brief Member function for resizing the buffer.
   * It's argument is the number of Ts that the buffer will contain after
   * the resing operation. If it's lower than the current number, the last
   * elements will be discarded.
   */
  void resize(std::ptrdiff_t to);

  /** \brief Returns the size of the buffer.
   * The return type is ptrdiff_t because if it was size_t then the compiler
   * would complain about narrowing if some math is performed on the result
   * then passed to the resize_buffer member function, for example.
   */
  std::ptrdiff_t size() const;

private:
  /** \brief Internal constructor for move operations. */
  buffer();

  /** \brief Internal constructor for resizing operations. */
  buffer(size_t size, GLenum flags);

  /** \brief Abstracts the creation of the buffer. */
  void create_buffer(size_t size, GLenum flags);

private:
  friend class vertex_array_object;

  template <typename U>
  friend void swap(buffer<U> &lhs, buffer<U> &rhs);

  GLuint buffer_id = 0;
  std::ptrdiff_t buffer_size = 0;
  GLenum creation_flags;
};

template <typename T>
void swap(buffer<T> &lhs, buffer<T> &rhs);
}
}

#include "opengl/buffer.inl"
