#pragma once
#include <GL/glew.h>
#include <boost/iterator/iterator_facade.hpp>
#include <boost/signals2.hpp>
#include <type_traits>

namespace game_engine {
namespace opengl {
class program;

/** \brief Enum for specifying the frequency of modification of the buffer.
 * buf_freq_access::mod_once = set once and then used.
 * buf_freq_access::mod_little = set once and used several times, several times.
 * buf_freq_access::mod_freq = set once and used once, continously.
 */
enum class buf_freq_access
{
  mod_once,
  mod_little,
  mod_freq
};

/** \brief Enum for specifying the kind of access of the buffer.
 * Self explanatory.
 */
enum class buf_kind_access
{
  read_app_write_gl,
  read_gl_write_app,
  read_gl_write_gl
};

/** \brief Base class for buffers.
 */
class buffer_base
{
};

/** \brief Template that represents a GL buffer that stores data of type T.
 */
template < typename T >
class buffer : public buffer_base
{
public:
  class buffer_accessor;

private:
  class buffer_iterator_base
  {
  public:
    buffer_iterator_base( const buffer& buf, std::ptrdiff_t offs );

    buffer_iterator_base( const buffer_iterator_base& other );
    buffer_iterator_base( buffer_iterator_base&& other );

    buffer_iterator_base& operator=( buffer_iterator_base other );

  protected:
    buffer_accessor& dereference( ) const;

    bool equal( const buffer_iterator_base& other ) const;

    void increment( );

    void decrement( );

    void advance( std::ptrdiff_t n );

    std::ptrdiff_t distance_to( const buffer_iterator_base& other ) const;

    mutable buffer_accessor buff_acc; // kinda hacky.

    friend void swap( buffer_iterator_base& lhs, buffer_iterator_base& rhs )
    {
      lhs.buff_acc.exchange( rhs.buff_acc );
    }

    friend boost::iterator_core_access;
  };

public:
  static_assert( std::is_trivial< T >::value,
                 "T must be trivial since we copy data byte by byte and"
                 "we don't call destructors" );

  /** \brief Proxy class that is returned from the dereference operation
   * of iterators. Abstracts uploading and downloading data from the GPU.
   * We want accessor1 = accessor2 to be equivalent to *accessor1 = *accessor2,
   * not to copy the accessor itself.
   */
  class buffer_accessor
  {
  public:
    buffer_accessor( const buffer< T >& buf, std::ptrdiff_t offs );

    buffer_accessor( );

    buffer_accessor& operator=( const T& data );

    operator T( ) const;

  private:
    friend class buffer_iterator_base;
    friend void swap( buffer_iterator_base& lhs, buffer_iterator_base& rhs );

    /** \brief A function that performs a task similar to that of swap, but it
     * swaps the accessors themselves instead of the contents pointed to.
     * It should only be used by buffer_iterator_base.
     */
    void exchange( buffer_accessor& rhs );

    const buffer* buff    = nullptr;
    std::ptrdiff_t offset = 0;
  };

  struct buffer_iterator
    : buffer_iterator_base,
      boost::iterator_facade< buffer_iterator, buffer_accessor, boost::random_access_traversal_tag >
  {
    friend struct const_buffer_iterator;
    using buffer_iterator_base::buffer_iterator_base;
  };

  struct const_buffer_iterator : buffer_iterator_base,
                                 boost::iterator_facade< const_buffer_iterator, const buffer_accessor,
                                                         boost::random_access_traversal_tag >
  {
    const_buffer_iterator( buffer_iterator other );

  public:
    using buffer_iterator_base::buffer_iterator_base;
  };

public:
  /** \brief Constructs a buffer of with size size, access frequency equal
   * to freq_acc, and type of access equal to kind_acc.
   */
  buffer( size_t size, buf_freq_access freq_acc, buf_kind_access kind_acc );

  buffer( buffer&& other );
  buffer& operator=( buffer&& other );

  ~buffer( );

  const buffer_accessor operator[]( std::ptrdiff_t index ) const;
  buffer_accessor operator[]( std::ptrdiff_t index );

  buffer_iterator begin( );
  const_buffer_iterator begin( ) const;
  const_buffer_iterator cbegin( ) const;

  buffer_iterator end( );
  const_buffer_iterator end( ) const;
  const_buffer_iterator cend( ) const;

  using iterator       = buffer_iterator;
  using const_iterator = const_buffer_iterator;

  using on_change_signal = boost::signals2::signal_type<
    void( void ), boost::signals2::keywords::mutex_type< boost::signals2::dummy_mutex > >::type;
  using on_change_slot = on_change_signal::slot_type;

  boost::signals2::scoped_connection on_change_connect( on_change_slot slot ) const;

  /** \brief Member function for resizing the buffer.
   * It's argument is the number of Ts that the buffer will contain after
   * the resing operation. If it's lower than the current number, the last
   * elements will be discarded.
   */
  void resize( std::ptrdiff_t to );

  /** \brief Returns the size of the buffer.
   * The return type is ptrdiff_t because if it was size_t then the compiler
   * would complain about narrowing if some math is performed on the result
   * then passed to the resize_buffer member function, for example.
   */
  std::ptrdiff_t size( ) const;

protected:
  /** \brief Returns the id of this buffer object. */
  GLuint get_buffer_id( ) const;

private:
  /** \brief Internal constructor for move operations. */
  buffer( );

  /** \brief Internal constructor for resizing operations. */
  buffer( size_t size, GLenum flags );

  /** \brief Abstracts the creation of the buffer. */
  void create_buffer( size_t size, GLenum flags );

private:
  friend class vertex_array_object;

  template < typename U >
  friend void swap( buffer< U >& lhs, buffer< U >& rhs );

  mutable on_change_signal change_signal;

  GLuint buffer_id           = 0;
  std::ptrdiff_t buffer_size = 0;
  GLenum creation_flags;
};

template < typename T >
void swap( buffer< T >& lhs, buffer< T >& rhs );
}
}

#include "opengl/buffer.inl"
