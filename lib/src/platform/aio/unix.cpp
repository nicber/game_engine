#include <aio/aio.h>
#include <algorithm>
#include <cassert>
#include <fcntl.h>
#include <logging/log.h>
#include <sstream>
#include <string.h>
#include <sys/ioctl.h>
#include <thr_queue/thread_api.h>
#include <unistd.h>

namespace game_engine {
namespace aio {
static boost::mutex queue_mutex;
static std::deque<aio_operation> op_queue;
thread_local bool aio_only_thread = false;

struct file_control_block {
  int fd;
  bool socket;
  bool op_inprogress;
  bool ready;
  omode omod;
};

file::file(const std::string &path, omode omod)
 : cblock(std::make_shared<file_control_block>())
{
  int flags;
  if (omod == omode::read_only) {
    flags = O_RDONLY;
  } else if (omod == omode::write_only) {
    flags = O_WRONLY;
  } else {
    flags = O_RDWR;
  }

  flags |= O_NONBLOCK;

  cblock->fd = open(path.c_str(), flags);
  if (cblock->fd == -1) {
    auto str = strerror(errno);
    std::ostringstream ss;
    ss << "Error when opening file: " << str;
    throw aio_runtime_error(ss.str());
  }

  cblock->socket = false;
  cblock->op_inprogress = false;
  cblock->ready = true;
  cblock->omod = omod;
}

static bool aio_type_compat_open_mode(aio_type otyp, omode omod) {
  if (omod == omode::read_write) {
    return true;
  } else if (otyp == aio_type::read && omod == omode::read_only) {
    return true;
  } else if (otyp == aio_type::write && omod == omode::write_only) {
    return true;
  }
  return false;
}

aio_result aio_operation::file_read()
{
  alloc_result_if_nec();
  assert(!control_block->socket);
  if (aio_only_thread) {
    ssize_t read_ret;
    do {
      read_ret = pread(control_block->fd, buf->buf, nbytes_min, offset);
    } while (read_ret == -1 && errno == EINTR);
    if (read_ret == -1) {
      std::ostringstream ss;
      ss << "Error when reading from file: " << strerror(errno);
      LOG() << ss.str();
      result->aio_except = aio_runtime_error(ss.str());
    } else {
      result->succeeded = true;
    }
  }
  return result;
}

aio_result aio_operation::file_write()
{
  alloc_result_if_nec();
  assert(!control_block->socket);
  if (aio_only_thread) {
    ssize_t write_ret;
    do {
      write_ret = pwrite(control_block->fd, buf->buf, nbytes_min, offset);
    } while (write_ret == -1 && errno == EINTR);
    if (write_ret == -1) {
      std::ostringstream ss;
      ss << "Error when writing to file: " << strerror(errno);
      LOG() << ss.str();
      result->aio_except = aio_runtime_error(ss.str());
    }
  }
  return result;
}

aio_result aio_operation::socket_read()
{
  alloc_result_if_nec();
  try_socket_read();
  if (!result->finished) {
    boost::lock_guard<boost::mutex> l(queue_mutex);
    op_queue.emplace_back(*this);
  }

  return result;
}

aio_result aio_operation::socket_write()
{
  alloc_result_if_nec();
  try_socket_write();
  if (!result->finished) {
    boost::lock_guard<boost::mutex> l(queue_mutex);
    op_queue.emplace_back(*this);
  }

  return result;
}

void
aio_operation::try_socket_read()
{
  assert(control_block->socket);
  assert(aio_type_compat_open_mode(type, control_block->omod));
  assert(type == aio_type::read);

  int available;
  size_t total_read_bytes = 0;
  ioctl(control_block->fd, FIONREAD, &available);
  if ((size_t) available >= nbytes_min) {
    result->finished = true;
    size_t to_read = std::min((size_t)available, nbytes_max);
    real_buffer = buf->buf;
    ssize_t read_ret;
    LOG() << "Trying to read directly" << to_read << " bytes out of "
          << available << " available ones.";
    do {
      read_ret = pread(control_block->fd, real_buffer, to_read, offset);

      if (read_ret == -1) {
        assert(errno != EWOULDBLOCK && errno != EAGAIN);
        if (errno == EINTR) {
          LOG() << "read() interrupted";
          continue;
        }
      } else {
        LOG() << "Successfully read " << read_ret;
        total_read_bytes += (size_t) read_ret;
        to_read -= (size_t) read_ret;
        real_buffer += read_ret;
        offset += read_ret;
      }
    } while (read_ret != -1 && to_read > 0);

    if (read_ret == -1) {
      result->succeeded = false;
      std::ostringstream ss;
      ss << "Error reading: " << strerror(errno);
      LOG() << ss.str();
      result->aio_except = aio_runtime_error(ss.str());
      return;
    } else {
      result->succeeded = true;
      result->read_bytes = total_read_bytes;
      return;
    }
  } else {
    LOG() << "Only " << available << " bytes available when the minimum is "
          << nbytes_min << " bytes.";
  }
}

void
aio_operation::try_socket_write()
{
  assert(control_block->socket);
  assert(aio_type_compat_open_mode(type, control_block->omod));
  assert(type == aio_type::write);

  size_t to_write = 0;
  real_buffer = real_buffer ? real_buffer : buf->buf;
  ssize_t write_ret;
  assert(!result->finished);

  do {
    write_ret = pwrite(control_block->fd, real_buffer, to_write, offset);
    if (write_ret != -1) {
      to_write -= (size_t) write_ret;
      real_buffer += write_ret;
      offset += write_ret;
      LOG() << "Successfully wrote " << write_ret << " bytes";
    } else if (write_ret == -1 && errno == EINTR) {
      LOG() << "write() interrupted";
      continue;
    }
  } while (write_ret != -1 && to_write > 0);

  if (errno == EAGAIN || errno == EWOULDBLOCK) {
    nbytes_min = to_write;
    return;
  } else {
    result->finished = true;
    if (to_write == 0) {
      result->succeeded = true;
      return;
    } else {
      result->succeeded = false;
      std::ostringstream ss;
      ss << "Error when writing to socket: " << strerror(errno);
      LOG() << ss.str();
      result->aio_except = aio_runtime_error(ss.str());
      return;
    }
  }
}

void
aio_operation::alloc_result_if_nec()
{
  if (!result) {
    result = std::make_shared<aio_result_t>();
    result->buf = buf;
  }
}

aio_result aio_operation::perform()
{
  if (type == aio_type::read) {
    if (control_block->socket) {
      return socket_read();
    } else {
      return file_read();
    }
  } else {
    if (control_block->socket) {
      return socket_write();
    } else {
      return file_write();
    }
  }
}
}
}
