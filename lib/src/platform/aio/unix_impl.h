#pragma once

#include <aio/aio.h>
#include <thr_queue/thread_api.h>

namespace game_engine {
namespace aio {
struct file_control_block {
  int fd;
  bool socket;
  bool op_inprogress;
  omode omod;
  std::vector<std::weak_ptr<aio_operation_t>> ongoing_operations;
};

extern boost::mutex socket_queue_mutex;
extern boost::mutex file_queue_mutex;
extern std::deque<aio_operation> socket_queue;
extern std::deque<aio_operation> file_queue;
extern thread_local bool aio_only_thread;

void notify_socket_queue_change();

void notify_file_queue_change();
}
}
