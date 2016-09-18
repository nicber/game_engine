#pragma once

#include "aio.h"
#include <thr_queue/event/cond_var.h>
#include <uv.h>

namespace game_engine {
namespace aio {
#define TCP_FAILURE( X )                                                                                     \
  struct tcp_##X##_failure : aio_runtime_error                                                               \
  {                                                                                                          \
    using aio_runtime_error::aio_runtime_error;                                                              \
  }

TCP_FAILURE( init );
TCP_FAILURE( bind );
TCP_FAILURE( listen );
TCP_FAILURE( connect );
TCP_FAILURE( read );

#undef TCP_FAILURE

class active_tcp_socket
{
public:
  struct connect_result
  {
    bool success;
    int status;
  };

  struct read_result
  {
    aio_buffer buf;
    ssize_t last_status;
    aio_buffer::size_type already_read = 0;
  };

  struct write_result
  {
    bool success;
    int status;
  };

  active_tcp_socket( );
  ~active_tcp_socket( );

  active_tcp_socket( active_tcp_socket&& other );
  active_tcp_socket& operator=( active_tcp_socket rhs );

  aio_operation< void > bind( uint16_t port );

  aio_operation< connect_result > connect( sockaddr_storage addr );

  aio_operation< read_result > read( aio_buffer::size_type min_read, aio_buffer::size_type max_read );

  aio_operation< write_result > write( aio_buffer buf );

private:
  aio_operation< write_result > write( std::vector< aio_buffer > buffers );

  struct private_constructor
  {
  };
  active_tcp_socket( private_constructor );

  struct read_internal_state
  {
    active_tcp_socket::read_result result;
    thr_queue::event::promise< read_result > prom;
    size_t min_read;
    size_t max_read;
  };

  struct data
  {
    uv_tcp_t socket;
    std::unique_ptr< read_internal_state > read_state;
    ssize_t read_error = 0;
    thr_queue::event::promise< void > closing_prom;
    ~data( ){};
  };

  std::shared_ptr< data > d;
  friend class passive_tcp_socket;
  friend struct write_internal_state;
  friend void swap( active_tcp_socket&, active_tcp_socket& ) noexcept;
};

void swap( active_tcp_socket& lhs, active_tcp_socket& rhs ) noexcept;

class passive_tcp_socket
{
public:
  struct accept_result
  {
    active_tcp_socket client_sock;
  };

  struct bind_listen_result
  {
    uint8_t backlog_size;
  };

  passive_tcp_socket( );
  ~passive_tcp_socket( );

  passive_tcp_socket( passive_tcp_socket&& other );
  passive_tcp_socket& operator=( passive_tcp_socket rhs );

  aio_operation< bind_listen_result > bind_and_listen( uint16_t port );

  aio_operation< accept_result > accept( );

private:
  struct private_constructor
  {
  };
  passive_tcp_socket( private_constructor );

  struct data
  {
    uv_tcp_t socket;
    uint8_t awaiting_accept = 0;
    thr_queue::event::promise< void > closing_prom;
    thr_queue::event::mutex mt;
    thr_queue::event::condition_variable cv;
  };
  std::shared_ptr< data > d;

  friend void swap( passive_tcp_socket&, passive_tcp_socket& ) noexcept;
};

void swap( passive_tcp_socket& lhs, passive_tcp_socket& rhs ) noexcept;
}
}
