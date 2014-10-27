#pragma once

#include <boost/strong_typedef.hpp>
#include "buffer.h"
#include "uniform.h"
#include <unordered_map>

namespace game_engine {
namespace opengl {
class uniform_buffer : public buffer<unsigned char> {
public:
  uniform_buffer(const program &prog, std::string block_name,
                 buf_freq_access freq_acc, buf_kind_access kind_acc);


  iterator begin(const std::string &name);
  const_iterator begin(const std::string &name) const;


private:
  buffer<unsigned char> construct_buffer_and_variables(const program &prog, const std::string &block_name,
                                         buf_freq_access freq_acc, buf_kind_access kind_acc);

private:
  std::string uniform_block_name;
  std::unordered_map<std::string, uniform> variables;
};

}
}
