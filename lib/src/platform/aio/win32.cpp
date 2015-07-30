#include <aio/aio.h>
#include <logging/log.h>
#include <thr_queue/thread_api.h>

namespace game_engine {
namespace aio {
struct win32_impl_t {
  win32_impl_t();
  ~win32_impl_t();
  HANDLE iocp = NULL;
};

win32_impl_t::win32_impl_t()
{
  assert(iocp == NULL);
  iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 0);

  WSADATA wsa_data;
  auto requested_version = MAKEWORD(2, 2);
  auto ret = WSAStartup(requested_version, &wsa_data);
  if (ret != 0 || wsa_data.wVersion != requested_version) {
    std::ostringstream ss; 
    ss << "Failed to initialize WinSock2 with version 2.2";
    LOG() << ss.str();
    WSACleanup();
    throw std::runtime_error(ss.str());
  }
  LOG() << "Successfully initialized WinSock2 version 2.2\n"
    << "Highest supported version: " << wsa_data.wHighVersion << '\n'
    << "Description: " << wsa_data.szDescription;
}

win32_impl_t::~win32_impl_t()
{
  WSACleanup();
  CloseHandle(iocp);
  iocp = NULL;
}

static win32_impl_t win32_impl;

struct file_control_block
{
  bool is_socket;
  omode open_mode;
  union {
    HANDLE file_handle;
    SOCKET socket;
  };
};

aio_result
aio_operation_t::perform()
{
  alloc_result_if_nec();
  if (type == aio_type::read) {
    // read socket.
    if (control_block->is_socket) {
      WSABUF buffer;
      buffer.buf = buf->buf;
      buffer.len = nbytes_min;
      assert(buf->len >= nbytes_min);
      DWORD flags = MSG_WAITALL;
      static_assert(sizeof(uint32_t) == sizeof(DWORD),
                    "needed for the reinterpret_cast on the next line");
      auto ret = WSARecv(control_block->socket, &buffer, 1,
                         reinterpret_cast<LPDWORD>(&(result->read_bytes)), 
                         &flags, &(result->overlapped), NULL);
      if (ret == 0) {
        LOG() << "Read operation on socket " << control_block->socket
          << " finished instantly";
        result->finished = true;
        return result;
      }
      auto error = WSAGetLastError();
      if (error == WSA_IO_PENDING) {
        LOG() << "Read operation on socket " << control_block->socket
          << " will complete asynchronously";
        return result;
      }
      result->finished = true;
      std::unique_ptr<char, decltype(LocalFree)> error_str(nullptr, LocalFree);

      std::ostringstream ss;
      auto format_err = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                                      NULL, error, 0, (LPSTR)error_str.get(), 0, NULL);
      if (format_err == 0) {
        ss << "Error formatting error message string. Error was " << error;
        LOG() << ss.str();
        auto except = aio_runtime_error(ss.str());
        result->aio_except = except;
        throw except;
      }

      ss << "Attempted to read from socket " << control_block->socket
        << " and got error: " << error_str.get();
      LOG() << ss.str();
      result->aio_except = aio_runtime_error(ss.str());
    }
    // read file.
    else {

    }
  }
}
}
}