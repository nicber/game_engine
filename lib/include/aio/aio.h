#pragma once

#include <atomic>
#include <boost/optional.hpp>
#include <memory>
#include <platform/aio.h>

namespace game_engine {
namespace aio {
struct aio_runtime_error : std::runtime_error {
  using std::runtime_error::runtime_error;
};

struct file_control_block;

enum class aio_type {
  read,
  write
};

struct aio_buffer {
  size_t len;
  char *buf;
  bool own;

  aio_buffer();
  aio_buffer(size_t len_, char *buf_);
  ~aio_buffer();
};

using aio_buffer_ptr = std::shared_ptr<aio_buffer>;

struct aio_result_t {
  std::atomic<bool> finished;
  boost::optional<aio_runtime_error> aio_except;
  aio_buffer_ptr buf;
  size_t read_bytes;

  aio_result_t();
};

using aio_result = std::shared_ptr<aio_result_t>;
void socket_thread();

class aio_operation_t : public std::enable_shared_from_this<aio_operation_t> {
public:
  aio_result perform();

private:
  aio_result file_read();
  aio_result file_write();
  aio_result socket_read();
  aio_result socket_write();
  void try_socket_read();
  void try_socket_write();
  void alloc_result_if_nec();

private:
  std::shared_ptr<file_control_block> control_block;
  aio_buffer_ptr buf;
  char *real_buffer;
  aio_type type;
  off_t offset;
  size_t nbytes_min;
  size_t nbytes_max;
  bool submitted;
  aio_result result;
  friend class file;
  friend void game_engine::aio::socket_thread();
};

using aio_operation   = std::shared_ptr<aio_operation_t>;
using aio_operation_w = std::weak_ptr<aio_operation_t>;

enum class omode {
  read_only,
  write_only,
  read_write
};

class file {
public:
  file(const std::string &path, omode omod = omode::read_only);

  aio_operation prepare_read(size_t length, aio_buffer_ptr to_bptr);
  aio_operation prepare_write(size_t length, aio_buffer_ptr to_bptr);

  aio_operation prepare_read_min_max(size_t min, size_t max, aio_buffer_ptr to_bptr);
private:
  std::shared_ptr<file_control_block> cblock;
};
}
}
