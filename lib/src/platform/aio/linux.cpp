#include "unix_impl.h"
#include <atomic>
#include <cstdint>
#include <logging/log.h>
#include <poll.h>
#include <queue>
#include <sys/eventfd.h>
#include <unordered_map>

namespace game_engine {
namespace aio {
struct linux_aio_threads_t {
  linux_aio_threads_t();
  ~linux_aio_threads_t();

  void remove_this_file_thread();

  std::vector<boost::thread> aio_threads;
  int socket_eventfd;
  std::atomic<bool> stopped;
  boost::condition_variable file_cv;
  int waiting_file_threads{0};
};

static linux_aio_threads_t linux_aio_threads;
static const int file_aio_threads_max = 32;

class pollfd_cont {
  public:
    pollfd *data();
    size_t size() const;

    void add(int fd, short events);
    void remove(int fd);

  private:
    using min_prio_queue = std::priority_queue<size_t,
                                               std::vector<size_t>,
                                               std::greater<size_t>>;
    std::vector<pollfd> pollfds;
    min_prio_queue unused_indexes;
};

pollfd *pollfd_cont::data()
{
  return pollfds.data();
}

size_t pollfd_cont::size() const
{
  return pollfds.size();
}

void pollfd_cont::add(int fd, short events)
{
  pollfd new_pollfd;
  new_pollfd.fd     = fd;
  new_pollfd.events = events;

  if (unused_indexes.empty()) {
    pollfds.emplace_back(std::move(new_pollfd));
  } else {
    auto index = unused_indexes.top();
    unused_indexes.pop();
    pollfds[index] = std::move(new_pollfd);
  }
}

void pollfd_cont::remove(int fd)
{
  if (unused_indexes.size() > (float) pollfds.size() * 3 / 4) {
    unused_indexes = min_prio_queue();
    std::vector<pollfd> new_data;
    new_data.reserve(pollfds.size() - unused_indexes.size());
    std::copy_if(begin(pollfds), end(pollfds), std::back_inserter(new_data),
      [fd] (auto &pfd) {
        return pfd.fd >= 0 && pfd.fd != fd;
      });
    pollfds = std::move(new_data);
  } else {
    auto it = std::find_if(begin(pollfds), end(pollfds), [fd] (auto &pfd) {
        return pfd.fd == fd;
      });

    assert(it != end(pollfds));

    it->fd     = -1;
    auto index = (size_t)std::distance(begin(pollfds), it);
    unused_indexes.push(index);
  }
}

void
socket_thread()
{
  struct fd_info {
    std::shared_ptr<file_control_block> fcb;
    bool read_added  = false;
    bool write_added = false;
  };

  aio_only_thread = true;
  std::unordered_map<int, fd_info> fd_info_map;
  pollfd_cont pfds;

  while (true) {
    std::deque<aio_operation> working_queue;
    {
      boost::lock_guard<boost::mutex> l(socket_queue_mutex);
      using std::swap;
      swap(working_queue, socket_queue);
    }
    for (auto &op : working_queue) {
      if (op->type == aio_type::read) {
        if (fd_info[op->control_block->fd].read_added) {
          std::ostringstream ss;
          ss << "A reading operation on this socket is already ongoing.";
          LOG() << ss.str();
          op->result->finished   = true;
          op->result->aio_except = aio_runtime_error(ss.str());
          continue;
        }

        auto &fd_inf      = fd_info_map[op->control_block->fd];
        fd_inf.fcb        = op->control_block;
        fd_inf.read_added = true;
        pfds.add(op->control_block->fd, POLLIN);
        auto &ongoing_operations_vec = op->control_block->ongoing_operations;
        auto it = std::find(begin(ongoing_operations_vec),
                            end(ongoing_operations_vec),
                            op);
        if (it == end(ongoing_operations_vec)) {
          ongoing_operations_vec.emplace_back(std::move(op));
        }
      } else {
        if (fd_info[op->control_block->fd].write_added) {
          
        }
      }
    }
  }
}

void
file_thread()
{
  aio_only_thread = true;

  while (true) {
    aio_operation op_to_perform;
    {
      boost::unique_lock<boost::mutex> l(file_queue_mutex);
      while (!linux_aio_threads.stopped && file_queue.empty()) {
        linux_aio_threads.waiting_file_threads++;
        auto notified = linux_aio_threads.file_cv.wait_for(l, boost::chrono::seconds(5));
        linux_aio_threads.waiting_file_threads--;
        if (notified == boost::cv_status::timeout) {
          linux_aio_threads.remove_this_file_thread();
          return;
        }
      }
      if (linux_aio_threads.stopped) {
        linux_aio_threads.remove_this_file_thread();
        return;
      }
      op_to_perform = std::move(file_queue.front());
      file_queue.pop_front();
    }
    op_to_perform->perform();
  }

}

linux_aio_threads_t::linux_aio_threads_t()
{
  stopped        = false;
  socket_eventfd = eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK);

  if (socket_eventfd == -1) {
    std::ostringstream ss;
    ss << "Error creating eventfd: " << strerror(errno);
    LOG() << ss.str();
    throw aio_runtime_error(ss.str());
  }

  aio_threads.emplace_back(socket_thread);
}

linux_aio_threads_t::~linux_aio_threads_t()
{
  stopped = true;
  notify_socket_queue_change();
  {
    boost::lock_guard<boost::mutex> l(file_queue_mutex);
    file_cv.notify_all();
  }

  for (auto &thr : aio_threads) {
    thr.detach();
  }
  close(socket_eventfd);
}

void
linux_aio_threads_t::remove_this_file_thread()
{
  auto this_id = boost::this_thread::get_id();
  auto it      = std::find_if(begin(aio_threads), end(aio_threads),
    [&](auto &thr) {
      return thr.get_id() == this_id;
    });
  assert(it != end(aio_threads));
  it->detach();
  aio_threads.erase(it);
}

void
notify_file_queue_change()
{
  boost::lock_guard<boost::mutex> l(file_queue_mutex);
  if (!linux_aio_threads.stopped) {
    if (linux_aio_threads.waiting_file_threads == 0
        && linux_aio_threads.aio_threads.size() < file_aio_threads_max) {
      linux_aio_threads.aio_threads.emplace_back(file_thread);
    }
  }
  linux_aio_threads.file_cv.notify_one();
}

void
notify_socket_queue_change()
{
  int64_t val = 1;
  ssize_t write_ret;

  do {
    write_ret = write(linux_aio_threads.socket_eventfd, &val, 8);
  } while (write_ret == -1 && errno == EINTR);

  if (write_ret == -1 && errno != EAGAIN) {
    std::ostringstream ss;
    ss << "Error writing to eventfd: " << strerror(errno);
    LOG() << ss.str();
    throw aio_runtime_error(ss.str());
  }
}
}
}

