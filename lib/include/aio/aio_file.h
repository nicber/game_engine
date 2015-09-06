#pragma once

#include "aio.h"

namespace game_engine {
namespace aio {
enum class omode {
  read_only,
  write_only,
  read_write
};

class read_file
{};

class write_file
{};

class file {
public:
  file(const std::string &path, omode omod = omode::read_only);

  aio_operation<read_file> prepare_read(size_t length, aio_buffer_ptr to_bptr);
  aio_operation<write_file> prepare_write(size_t length, aio_buffer_ptr to_bptr);
public:
  struct file_control_block;

private:
  std::shared_ptr<file_control_block> cblock;
};
}
}
