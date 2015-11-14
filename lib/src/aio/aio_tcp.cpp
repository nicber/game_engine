#include <aio/aio_tcp.h>
#include "../thr_queue/event/uv_thread.h"
#include <thr_queue/util_queue.h>

namespace game_engine {
namespace aio {
passive_tcp_socket::passive_tcp_socket()
  :d(std::make_shared<data>())
{
  using namespace thr_queue::event;
  uv_thr_cor_do<void>([d = d](auto prom) {
    if (int err = uv_tcp_init(uv_default_loop(), &d->socket)) {
      std::ostringstream ss;
      ss << "uv_tcp_init: " << uv_strerror(err);
      LOG() << ss.str();
      prom.set_exception(aio_runtime_error(ss.str()));
    };

    d->socket.data = d.get();
    prom.set_value();
  }).wait();
}

passive_tcp_socket::~passive_tcp_socket()
{
  using namespace thr_queue::event;
  if (!d) {
    return;
  }
  uv_thr_cor_do<void>([&] (auto prom)  {
    d->closing_prom = std::move(prom);
    uv_close((uv_handle_t *)&d->socket, [] (uv_handle_t *h) {
      ((data *)h->data)->closing_prom.set_value();
    });
  }).wait();
}

passive_tcp_socket::passive_tcp_socket(passive_tcp_socket &&other)
 :passive_tcp_socket(private_constructor())
{
  swap(*this, other);
}

passive_tcp_socket &passive_tcp_socket::operator=(passive_tcp_socket rhs)
{
  swap(*this, rhs);
  return *this;
}

passive_tcp_socket::passive_tcp_socket(passive_tcp_socket::private_constructor)
 :d(nullptr)
{}

void swap(passive_tcp_socket &lhs, passive_tcp_socket &rhs) noexcept
{
  swap(lhs.d, rhs.d);
}

aio_operation<passive_tcp_socket::bind_listen_result>
passive_tcp_socket::bind_and_listen(uint16_t port)
{
  using namespace thr_queue::event;

  return make_aio_operation([d_lambda = d, port]() mutable {
    auto d = std::move(d_lambda);
    auto uv_code = [d_l = std::move(d), port](auto prom) {
      sockaddr_in addr;
      uv_ip4_addr("0.0.0.0", port, &addr);

      if (int err = uv_tcp_bind(&d_l->socket, (sockaddr *)&addr, 0)) {
        std::ostringstream ss;
        ss << "uv_tcp_bind: " << uv_strerror(err);
        LOG() << ss.str();
        prom.set_exception(aio_runtime_error(ss.str()));
        return;
      }

      auto listen_cb = [](uv_stream_t* stream, int status) {
        //TODO: handle status < 0.
        auto &data = *static_cast<passive_tcp_socket::data*>(stream->data);
        assert(status == 0);
        thr_queue::default_par_queue().submit_work([&] {
          boost::lock_guard<thr_queue::event::mutex> lock(data.mt);
          ++data.awaiting_accept;
          data.cv.notify();
        });
      };

      const uint8_t backlog_size = 10;
      if (int err = uv_listen((uv_stream_t*)&d_l->socket, backlog_size, listen_cb)) {
        std::ostringstream ss;
        ss << "uv_listen: " << uv_strerror(err);
        LOG() << ss.str();
        prom.set_exception(aio_runtime_error(ss.str()));
        return;
      }

      prom.set_value(bind_listen_result{backlog_size});
    };

    return uv_thr_cor_do<bind_listen_result>(std::move(uv_code));
  });
}
aio_operation<passive_tcp_socket::accept_result>
passive_tcp_socket::accept()
{
  using namespace thr_queue::event;
  return make_aio_operation([d_l = d]() mutable {
    promise<accept_result> prom;
    auto fut = prom.get_future();

    auto d = std::move(d_l);
    thr_queue::default_par_queue().submit_work(
    [d_l = std::move(d), prom = std::move(prom)]() mutable {
      accept_result proposed_result;
      bool successful_accept;
      do {
        auto lock = boost::unique_lock<thr_queue::event::mutex>(d_l->mt);
        while (d_l->awaiting_accept == 0) {
          d_l->cv.wait(lock);
        }
        --d_l->awaiting_accept;
        lock.unlock();
        successful_accept = uv_thr_cor_do<bool>([&socket = d_l->socket, &proposed_result](auto prom) {
          if (int err = uv_accept((uv_stream_t*)&socket,
              (uv_stream_t*)&proposed_result.client_sock.d->socket)) {
            LOG() << "uv_accept: " << uv_strerror(err);
            prom.set_value(false);
            return;
          }
          prom.set_value(true);
        }).get();
      } while (!successful_accept);
      prom.set_value(std::move(proposed_result));
    });

    return fut;
  });
}

active_tcp_socket::active_tcp_socket()
  :d(std::make_shared<data>())
{
  using namespace thr_queue::event;
  uv_thr_cor_do<void>([d = d](auto prom) {
    if (int err = uv_tcp_init(uv_default_loop(), &d->socket)) {
      std::ostringstream ss;
      ss << "uv_tcp_init: " << uv_strerror(err);
      LOG() << ss.str();
      prom.set_exception(aio_runtime_error(ss.str()));
      return;
    }
    d->socket.data = d.get();
    prom.set_value();
  }).wait();
}

active_tcp_socket::~active_tcp_socket()
{
  if (!d) {
    return;
  }
  assert(!d->read_state);
  thr_queue::event::uv_thr_cor_do<void>([d = d](auto prom) {
    d->closing_prom = std::move(prom);
    uv_close((uv_handle_t*)&d->socket, [] (uv_handle_t *handle) {
      ((data *)handle->data)->closing_prom.set_value();
    });
  }).wait();
}

active_tcp_socket::active_tcp_socket(active_tcp_socket &&other)
 :active_tcp_socket(private_constructor())
{
  swap(*this, other);
}

active_tcp_socket&
active_tcp_socket::operator=(active_tcp_socket rhs)
{
  swap(*this, rhs);
  return *this;
}
aio_operation<active_tcp_socket::bind_result>
active_tcp_socket::bind(uint16_t port)
{
  using namespace thr_queue::event;
  return make_aio_operation([d = d, port]() mutable {
    auto d_l = std::move(d);
    return uv_thr_cor_do<bind_result>([d = std::move(d_l), port](auto prom) {
      sockaddr_in addr;
      uv_ip4_addr("0.0.0.0", port, &addr);
      if (int err = uv_tcp_bind(&d->socket, (sockaddr *)&addr, 0)) {
        std::ostringstream ss;
        ss << "uv_tcp_bind: " << uv_strerror(err);
        LOG() << ss.str();
        prom.set_exception(aio_runtime_error(ss.str()));
        return;
      }
      prom.set_value(bind_result{});
    });
  });
}

aio_operation<active_tcp_socket::connect_result>
active_tcp_socket::connect(sockaddr_storage addr)
{
  using namespace thr_queue::event;
  return make_aio_operation([d = d, addr]() mutable {
    auto dl = std::move(d);
    return uv_thr_cor_do<connect_result>([d = std::move(dl), addr](auto prom) {
      using con_req_prom_pair = std::pair<uv_connect_t, thr_queue::event::promise<connect_result>>;
      static_assert(std::is_standard_layout<con_req_prom_pair>::value
                    && offsetof(con_req_prom_pair, first) == 0, "");

      auto fut = prom.get_future();
      auto uv_conn_req = new con_req_prom_pair(uv_connect_t{}, std::move(prom));

      auto connect_cb = [](uv_connect_t* req_and_prom, int status) {
        connect_result result;
        std::unique_ptr<con_req_prom_pair> req_ptr(reinterpret_cast<con_req_prom_pair*>(req_and_prom));
        if (status != 0) {
          std::ostringstream ss;
          ss << "result of connect(): " << uv_strerror(status);
          LOG() << ss.str();
        }
        result.success = status == 0;
        result.status = status;
        req_ptr->second.set_value(std::move(result));
      };

      if (int err = uv_tcp_connect(&uv_conn_req->first, &d->socket,
          (const sockaddr*)&addr, connect_cb)) {
        std::ostringstream ss;
        ss << "uv_tcp_connect: " << uv_strerror(err);
        LOG() << ss.str();
        uv_conn_req->second.set_exception(aio_runtime_error(ss.str()));
        delete uv_conn_req;
        return fut;
      }

      return fut;
    });
  });
}


aio_operation<active_tcp_socket::read_result>
active_tcp_socket::read(aio_buffer::size_type min_read, aio_buffer::size_type max_read)
{
  return make_aio_operation([d = d, max_read, min_read]() mutable {
    auto dl = std::move(d);
    return thr_queue::event::uv_thr_cor_do<read_result>(
      [d = std::move(dl), max_read, min_read](auto prom) {
      if (d->read_state) {
        prom.set_exception(std::logic_error("a read is already going on"));
        return;
      }

      d->read_state = std::make_unique<read_internal_state>();
      d->read_state->prom = std::move(prom);
      d->read_state->max_read = max_read;
      d->read_state->min_read = min_read;

      try {
        d->read_state->result.buf = aio_buffer(max_read);
      } catch (std::exception &) {
        prom.set_exception(std::current_exception());
        return;
      }

      auto alloc_cb = [](uv_handle_t* handle, size_t, uv_buf_t* buf) {
        auto data_ptr = (data *)handle->data;
        auto already_read = data_ptr->read_state->result.already_read;
        *buf = data_ptr->read_state->result.buf.get_subbuffer(already_read);
      };

      auto read_cb = [](uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf) {
        auto data_ptr = (data *)stream->data;
        auto &read_state = data_ptr->read_state;
        bool stop_read = false;

        read_state->result.last_status = nread;
        if (nread > 0) {
          read_state->result.already_read += (aio_buffer::size_type) nread;
          if (read_state->result.already_read >= read_state->min_read) {
            stop_read = true;
          }
        } else if (nread == 0) {
          // EAGAIN or EWOULDBLOCK
        } else {
          data_ptr->read_error = nread;
          stop_read = true;
        }

        if (stop_read) {
          uv_read_stop((uv_stream_t*)&data_ptr->socket);
          read_state->prom.set_value(std::move(read_state->result));
          read_state.reset();
        }
      };

      uv_read_start((uv_stream_t*)&d->socket, alloc_cb, read_cb);
      int uv_read_ret = uv_read_start((uv_stream_t*)&d->socket, alloc_cb, read_cb);
      if (uv_read_ret < 0) {
        std::ostringstream ss;
        ss << "uv_read callback: " << uv_strerror(uv_read_ret);
        LOG() << ss.str();
        prom.set_exception(aio_runtime_error(ss.str()));
      }
    });
  });
}

aio_operation<active_tcp_socket::write_result>
active_tcp_socket::write(aio_buffer buf)
{
  std::vector<aio_buffer> bufs;
  bufs.emplace_back(std::move(buf));
  return write(std::move(bufs));
}

struct write_internal_state {
  std::shared_ptr<active_tcp_socket::data> keep_data_alive;
  std::vector<aio_buffer> bufs;
  thr_queue::event::promise<active_tcp_socket::write_result> prom;
};

aio_operation<active_tcp_socket::write_result>
active_tcp_socket::write(std::vector<aio_buffer> buffers)
{
  assert(d);
  return make_aio_operation([buffers = std::move(buffers), d = d]() mutable {
    auto dl = std::move(d);
    auto bufs = std::move(buffers);
    return thr_queue::event::uv_thr_cor_do<write_result>(
      [buffers = std::move(bufs), d = std::move(dl)](auto prom) mutable {
      uv_write_t *write_req = new uv_write_t;
      auto *bufs_ptr = buffers.data();
      auto nbufs = buffers.size();
      auto socket_ptr = &d->socket;
      write_req->data = new write_internal_state{std::move(d),
                                                 std::move(buffers),
                                                 std::move(prom)};

      auto write_cb = [](uv_write_t* req, int status) {
        auto &istate = *(write_internal_state*)req->data;
        if (status != 0) {
          LOG() << "uv_write callback: " << uv_strerror(status);
        }
        istate.prom.set_value({status == 0, status});
        if (istate.keep_data_alive.use_count() == 1) {
          LOG() << "WARNING: write request is keeping socket alive.";
        }
        delete &istate;
        delete req;
      };

      uv_write(write_req, (uv_stream_t*)socket_ptr, bufs_ptr, nbufs, write_cb);
    });
  });
}

active_tcp_socket::active_tcp_socket(private_constructor)
 :d(nullptr) {}

void
swap(active_tcp_socket &lhs, active_tcp_socket &rhs) noexcept
{
  swap(lhs.d, rhs.d);
}
}
}
