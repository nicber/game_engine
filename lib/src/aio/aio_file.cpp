#include <aio/aio_file.h>

namespace game_engine {
namespace aio {
struct aio_operation_read_file : aio_operation_t<read_file> {
  aio_result_promise<read_file> prom;
  aio_result_future<read_file> do_perform() final override
  {
    return prom.get_future();
  }
};

struct aio_operation_write_file : aio_operation_t<write_file> {
  aio_result_promise<write_file> prom;
  aio_result_future<write_file> do_perform() final override
  {
    return prom.get_future();
  }
};

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