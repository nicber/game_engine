#include <aio/aio_file.h>

namespace game_engine {
namespace aio {

file::file(const std::string & path, omode omod)
{
}

aio_operation<read_file> file::prepare_read(size_t length, aio_buffer_ptr to_bptr)
{
  return aio_operation<read_file>();
}

aio_operation<write_file> file::prepare_write(size_t length, aio_buffer_ptr to_bptr)
{
  return aio_operation<write_file>();
}
}
}