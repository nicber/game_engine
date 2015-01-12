#pragma once

#include <boost/strong_typedef.hpp>
#include "buffer.h"
#include "uniform.h"

namespace game_engine {
namespace opengl {
struct uniform_buffer_data {
  std::vector<uniform> variables;
};

/** \brief Compares uniforms by their name field only. */
bool uniform_buff_variable_sort_func(const uniform &lhs, const uniform &rhs);

/** \brief Compares a uniform and a string that stores a uniform name. */
bool uniform_buff_variable_sort_with_name_func(const uniform &lhs, const std::string &rhs);

/** \brief Utility for constructing the buffer base class and the data member at the same time. */
struct buffer_and_data_constructor {
  buffer_and_data_constructor(const program &prog, const std::string &name);
  std::shared_ptr<uniform_buffer_data> data;
  size_t block_size;
};

/** \brief Subclass of buffer<unsigned char> that implements a uniform buffer.
 * It implements utility functions for setting a specific variable of the
 * uniform block stored by this function.
 */
class uniform_buffer : buffer_and_data_constructor, public buffer<unsigned char> {
public:
  /** \brief Constructs a uniform buffer that stores a uniform block compatible
   * with the one named 'block_name' of the program 'prog'.
   * 'freq_acc' and 'kind_acc' specify buffer specific options. See buffer<T> for
   * more information.
   */
  uniform_buffer(const program &prog, const std::string &block_name,
                 buf_freq_access freq_acc, buf_kind_access kind_acc);

  /** \brief Returns an iterator to the start of the variable identified by name. */
  iterator begin(const std::string &name);
  const_iterator begin(const std::string &name) const;
  const_iterator cbegin(const std::string &name) const;

  void bind_to(uniform_block_binding_handle handle);
  /** \brief Utility member function for constructing the buffer base class. */
};

}
}
