#include "../thr_queue/event/uv_thread.h"
#include <aio/aio_file.h>
#include <boost/scope_exit.hpp>
#include <thr_queue/event/future.h>
#include <thr_queue/util_queue.h>

namespace game_engine {
namespace aio {
static int
access_and_mode_to_flags(file_access access, file_mode mode)
{
  int flags = 0;
  switch (access) {
    default:
      assert(false);
    case file_access::read_only:
      flags |= O_RDONLY;
      break;
    case file_access::read_write:
      flags |= O_RDWR;
      break;
    case file_access::write_only:
      flags |= O_WRONLY;
      break;
    case file_access::write_append:
      flags |= O_WRONLY | O_APPEND;
      break;
    case file_access::read_write_append:
      flags |= O_RDWR | O_APPEND;
      break;
  }
  switch (mode) {
    default:
      assert(false);
    case file_mode::create_or_open:
      flags |= O_CREAT;
      break;
    case file_mode::create_or_truncate:
      flags |= O_CREAT | O_TRUNC;
      break;
    case file_mode::truncate:
      flags |= O_TRUNC;
      break;
    case file_mode::open_existing:
      break;
  }
  return flags;
}

struct file::file_control_block {
  const int fd;
  const file_access access;
  const file_mode mode;

  thr_queue::event::mutex mt;
  thr_queue::event::condition_variable cv;
  std::atomic<int8_t> ongoing_operations {0}; // if -1 then it is closing.
                                              // -2 means it is closed.
public:
  file_control_block(int f, file_access ac, file_mode m)
   :fd(f)
   ,access(ac)
   ,mode(m)
  {}

  bool increment_counter() {
    boost::lock_guard<thr_queue::event::mutex> l(mt);
    assert(ongoing_operations < 127 && "ongoing_operations would overflow");
    if (ongoing_operations == -1) {
      return false;
    } else {
      ongoing_operations++;
      return true;
    }
  }

  void decrement_counter() {
    boost::lock_guard<thr_queue::event::mutex> l(mt);
    assert(ongoing_operations > 0 && "there must be an ongoing_operation");
    ongoing_operations--;
    if (ongoing_operations == 0) {
      cv.notify(); // notify someone waiting to close.
    }
  }

  bool wait_and_set_closing() {
    boost::unique_lock<thr_queue::event::mutex> l(mt);
    if (ongoing_operations <= -1) {
      return false;
    }
    while(ongoing_operations > 0) {
      cv.wait(l);
    }
    ongoing_operations = -1;
    return true;
  }
};

file::~file()
{
  if (!cblock) {
    return;
  }

  if (cblock->ongoing_operations == -2) {
    return;
  }

  auto close_fut = close(*this)->perform();
  close_fut.wait();
  auto excpt = close_fut.get_exception();
  if (excpt != nullptr) {
    try {
      std::rethrow_exception(excpt);
    } catch (std::exception &e) {
      LOG() << "close() failed: " << e.what();
    }
  }
}

file::file(fcb_shr_ptr fcb_ptr)
 :cblock(std::move(fcb_ptr))
{}

template <typename T>
struct fcb_req_wrapper_storage {
  T data;

  fcb_req_wrapper_storage(T dat)
   :data(std::move(dat))
  {}
};

template <>
struct fcb_req_wrapper_storage<void> {
};

template <typename T, typename U = void>
struct fcb_req_wrapper : fcb_req_wrapper_storage<U>
{
private:
  struct priv_struct{};

public:
  file::fcb_shr_ptr fcb_ptr;
  thr_queue::event::promise<T> init_prom;
  uv_fs_t req;

  template <typename V = U>
  static std::shared_ptr<fcb_req_wrapper<T, U>>
  create_fcb_req_wrapper(
      std::enable_if_t<!std::is_same<V, void>::value, file::fcb_shr_ptr> ptr,
      thr_queue::event::promise<T> prom, V data)
  {
    auto res_ptr = std::make_shared<fcb_req_wrapper<T, U>>(priv_struct(),
        std::move(ptr), std::move(prom), std::move(data));

    res_ptr->req.data = new std::shared_ptr<fcb_req_wrapper<T, U>>(res_ptr);
    return res_ptr;
  }

  template <typename V = U>
  static std::shared_ptr<fcb_req_wrapper<T, U>>
  create_fcb_req_wrapper(
      std::enable_if_t<std::is_same<V, void>::value, file::fcb_shr_ptr> ptr,
      thr_queue::event::promise<T> prom)
  {
    auto res_ptr = std::make_shared<fcb_req_wrapper<T, U>>(priv_struct(),
        std::move(ptr), std::move(prom));

    res_ptr->req.data = new std::shared_ptr<fcb_req_wrapper<T, U>>(res_ptr);
    return res_ptr;
  }

  template <typename V = U>
  fcb_req_wrapper(priv_struct, std::enable_if_t<std::is_same<V, void>::value,
      file::fcb_shr_ptr> ptr, thr_queue::event::promise<T> prom)
   :fcb_req_wrapper_storage<U>()
   ,fcb_ptr(std::move(ptr))
   ,init_prom(std::move(prom))
  {}

  template <typename V = U>
  fcb_req_wrapper(priv_struct, std::enable_if_t<!std::is_same<V, void>::value,
      file::fcb_shr_ptr> ptr, thr_queue::event::promise<T> prom, V data)
   :fcb_req_wrapper_storage<U>(std::move(data))
   ,fcb_ptr(std::move(ptr))
   ,init_prom(std::move(prom))
  {}

  fcb_req_wrapper(fcb_req_wrapper&&) = delete;
  fcb_req_wrapper(const fcb_req_wrapper&) = delete;

  fcb_req_wrapper& operator=(fcb_req_wrapper&&) = delete;
  fcb_req_wrapper& operator=(const fcb_req_wrapper&) = delete;

  ~fcb_req_wrapper()
  {
    if (req.data != nullptr) {
      LOG() << "req.data contains a shared pointer that creates a reference"
            << " cycle. Leaking memory";
    }
  }
};

template<typename T>
std::shared_ptr<T>
get_fcb_req_wrapper_from_req(uv_fs_t* req, bool move)
{
  auto shr_ptr_ptr = static_cast<std::shared_ptr<T>*>(req->data);
  if (move) {
    auto fcb_wrapper_ptr = std::move(*shr_ptr_ptr);
    delete shr_ptr_ptr;
    req->data = nullptr;
    return fcb_wrapper_ptr;
  } else {
    return *shr_ptr_ptr;
  }
}

aio_operation<file>
open(path p, file_access access, file_mode mode)
{
  return make_aio_operation([=, p = std::move(p)]() {
    using access_mode_pair = std::pair<file_access, file_mode>;
    using fcb_init_struct = fcb_req_wrapper<file, access_mode_pair>;
    auto uv_code = [=, p = std::move(p)] (auto prom) {
      auto open_cb = [](uv_fs_t* req) {
        auto fcb_is_ptr =
          get_fcb_req_wrapper_from_req<fcb_init_struct>(req, true);

        BOOST_SCOPE_EXIT_ALL(req, fcb_is_ptr) {
          uv_fs_req_cleanup(req);
        };

        int fd = req->result;
        if (fd == -1) {
          fcb_is_ptr->init_prom.set_exception(file_open_failure(fd,
                "uv_fs_open"));
        } else {
          auto new_fcb = std::make_shared<file::file_control_block>(fd,
              fcb_is_ptr->data.first, fcb_is_ptr->data.second);
          fcb_is_ptr->init_prom.set_value(std::move(new_fcb));
        }
      };

      auto fcb_init_struct_ptr =
        fcb_init_struct::create_fcb_req_wrapper(nullptr,std::move(prom),
            {access, mode});

      int flags = access_and_mode_to_flags(access, mode);
      uv_fs_open(uv_default_loop(), &fcb_init_struct_ptr->req, p.c_str(),
          flags, 0x664, open_cb);
    };
    return thr_queue::event::uv_thr_cor_do<file>(std::move(uv_code));
  });
}

aio_operation<read_result>
read(file &file, size_t quantity, int64_t offset)
{
  return make_aio_operation([=, cblock = file.cblock] {
    if (!cblock->increment_counter()) {
      auto excpt = file_read_failure(-EBADF, "read: already closed");
      auto fut = thr_queue::event::future_with_exception<read_result>(std::move(excpt));
      return fut;
    }

    auto uv_code = [=, cblock = std::move(cblock)](auto prom) {
      using fcb_read_struct = fcb_req_wrapper<read_result, read_result>;
      auto fcb_read_struct_ptr =
      fcb_read_struct::create_fcb_req_wrapper(std::move(cblock),
          std::move(prom), read_result{{quantity}, 0});

      auto read_cb = [] (uv_fs_t *req) {
        auto fcb_read_struct_ptr =
          get_fcb_req_wrapper_from_req<fcb_read_struct>(req, true);

        BOOST_SCOPE_EXIT_ALL(req, fcb_read_struct_ptr) {
          uv_fs_req_cleanup(req);
          thr_queue::default_par_queue().submit_work([=] {
            fcb_read_struct_ptr->fcb_ptr->decrement_counter();
          });
        };

        if (req->result < 0) {
          std::ostringstream ss;
          ss << "uv_fs_read: " << uv_strerror(req->result);
          LOG() << ss.str();
          fcb_read_struct_ptr->init_prom
            .set_exception(file_read_failure(req->result, "uv_fs_read"));
          return;
        } else {
          auto rresult = std::move(fcb_read_struct_ptr->data);
          rresult.read_total = req->result;
          fcb_read_struct_ptr->init_prom.set_value(std::move(rresult));
          return;
        }
      };

      uv_fs_read(uv_default_loop(), &fcb_read_struct_ptr->req,
          fcb_read_struct_ptr->fcb_ptr->fd,
          &fcb_read_struct_ptr->data.buf, 1,
          offset, read_cb);
    };
    return thr_queue::event::uv_thr_cor_do<read_result>(std::move(uv_code));
  });
}

aio_operation<write_result>
write(file &file, aio_buffer buf, int64_t offset)
{
  return make_aio_operation([offset, cblock = file.cblock, buf = std::move(buf)] () mutable {
    if (!cblock->increment_counter()) {
      auto excpt = file_write_failure(-EBADF, "write: already closed");
      return thr_queue::event::future_with_exception<write_result>(std::move(excpt));
    }

    auto uv_code = [=, cblock = std::move(cblock), buf = std::move(buf)](auto prom) mutable {
      using fcb_write_struct = fcb_req_wrapper<write_result, aio_buffer>;
      auto fcb_write_struct_ptr =
      fcb_write_struct::create_fcb_req_wrapper(std::move(cblock),
          std::move(prom), std::move(buf));

      auto write_cb = [] (uv_fs_t *req) {
        auto fcb_write_struct_ptr =
          get_fcb_req_wrapper_from_req<fcb_write_struct>(req, true);

        BOOST_SCOPE_EXIT_ALL(req, fcb_write_struct_ptr) {
          uv_fs_req_cleanup(req);
          thr_queue::default_par_queue().submit_work([=] {
            fcb_write_struct_ptr->fcb_ptr->decrement_counter();
          });
        };

        if (req->result < 0) {
          fcb_write_struct_ptr->init_prom
            .set_exception(file_write_failure(req->result, "uv_fs_write"));
          return;
        } else {
          fcb_write_struct_ptr->init_prom.set_value({req->result});
          return;
        }
      };

      uv_fs_write(uv_default_loop(), &fcb_write_struct_ptr->req,
          fcb_write_struct_ptr->fcb_ptr->fd, &fcb_write_struct_ptr->data, 1,
          offset, write_cb);
    };
    return thr_queue::event::uv_thr_cor_do<write_result>(std::move(uv_code));
  });
}

aio_operation<void>
truncate(file &file, int64_t offset)
{
  return make_aio_operation([offset, cblock = file.cblock] () mutable {
    if (!cblock->increment_counter()) {
      auto excpt = file_truncate_failure(-EBADF, "truncate: already closed");
      return thr_queue::event::future_with_exception<void>(std::move(excpt));
    }

    auto uv_code = [=, cblock = std::move(cblock)] (auto prom) mutable {
      using fcb_trunc_struct = fcb_req_wrapper<void, void>;
      auto fcb_trunc_struct_ptr = fcb_trunc_struct::create_fcb_req_wrapper(std::move(cblock), std::move(prom));

      auto truncate_cb = [] (uv_fs_t *req) {
        auto fcb_trunc_struct_ptr =
          get_fcb_req_wrapper_from_req<fcb_trunc_struct>(req, true);

        BOOST_SCOPE_EXIT_ALL(req, fcb_trunc_struct_ptr) {
          uv_fs_req_cleanup(req);
          thr_queue::default_par_queue().submit_work([=] {
            fcb_trunc_struct_ptr->fcb_ptr->decrement_counter();
          });
        };

        if (req->result < 0) {
          fcb_trunc_struct_ptr->init_prom
            .set_exception(file_truncate_failure(req->result, "uv_fs_truncate"));
          return;
        } else {
          fcb_trunc_struct_ptr->init_prom.set_value();
          return;
        }
      };

      uv_fs_ftruncate(uv_default_loop(), &fcb_trunc_struct_ptr->req,
          fcb_trunc_struct_ptr->fcb_ptr->fd, offset, truncate_cb);
    };
    return thr_queue::event::uv_thr_cor_do<void>(std::move(uv_code));
  });
}

aio_operation<void>
close(file & fil)
{
  return make_aio_operation([cblock = fil.cblock] (perform_helper<void> &ph) mutable {
    if (!cblock->wait_and_set_closing()) {
        thr_queue::event::promise<void> prom;
        ph.set_future(prom.get_future());
        boost::unique_lock<thr_queue::event::mutex> l(cblock->mt);
        while(cblock->ongoing_operations != -2) {
          assert(cblock->ongoing_operations == -1);
          cblock->cv.wait(l);
        }
        prom.set_value();
    }

    auto uv_code = [=, cblock = std::move(cblock)] (auto prom) mutable {
      using fcb_close_struct = fcb_req_wrapper<void, void>;
      auto fcb_close_struct_ptr =
      fcb_close_struct::create_fcb_req_wrapper(std::move(cblock),
          std::move(prom));

      auto close_cb = [] (uv_fs_t *req) {
        auto fcb_close_struct_ptr =
          get_fcb_req_wrapper_from_req<fcb_close_struct>(req, true);

        BOOST_SCOPE_EXIT_ALL(req) {
          uv_fs_req_cleanup(req);
        };

        thr_queue::default_par_queue().submit_work([result = req->result, fcb_close_struct_ptr = std::move(fcb_close_struct_ptr)] {
          auto &fcb = *fcb_close_struct_ptr->fcb_ptr;

          boost::unique_lock<thr_queue::event::mutex> l(fcb.mt);
          assert(fcb.ongoing_operations == -1);
          fcb.ongoing_operations = -2;
          if (result < 0) {
            fcb_close_struct_ptr->init_prom
              .set_exception(file_close_failure(result, "uv_fs_close"));
            return;
          } else {
            fcb_close_struct_ptr->init_prom.set_value();
            return;
          }
          fcb.cv.notify();
        });
      };

      uv_fs_close(uv_default_loop(), &fcb_close_struct_ptr->req,
          fcb_close_struct_ptr->fcb_ptr->fd, close_cb);
    };
    ph.set_future(thr_queue::event::uv_thr_cor_do<void>(std::move(uv_code)));
  });
}

aio_operation<void>
unlink(path p)
{
  return make_aio_operation([p = std::move(p)] () mutable {
    auto uv_code = [p = std::move(p)](auto prom) mutable {
      using unlink_struct = fcb_req_wrapper<void, void>;
      auto unlink_struct_ptr = unlink_struct::create_fcb_req_wrapper(nullptr,
          std::move(prom));

      auto unlink_cb = [] (uv_fs_t *req) {
        auto req_struct_ptr =
          get_fcb_req_wrapper_from_req<unlink_struct>(req, true);

        BOOST_SCOPE_EXIT_ALL(req) {
          uv_fs_req_cleanup(req);
        };

        if (req->result < 0) {
          req_struct_ptr->init_prom
            .set_exception(file_unlink_failure(req->result, "uv_fs_unlink"));
          return;
        } else {
          req_struct_ptr->init_prom.set_value();
          return;
        }
      };

      uv_fs_unlink(uv_default_loop(), &unlink_struct_ptr->req, p.c_str(),
          unlink_cb);
    };
    return thr_queue::event::uv_thr_cor_do<void>(std::move(uv_code));
  });
}

#define STAT_COMMON(ptr, OP, EXTRA_ARGS...)                                        \
      using stat_struct = fcb_req_wrapper<stat_result, void>;                 \
      auto stat_struct_ptr = stat_struct::create_fcb_req_wrapper(ptr,         \
          std::move(prom));                                                   \
                                                                              \
      auto stat_cb = [] (uv_fs_t *req) {                                      \
        auto stat_struct_ptr =                                                \
          get_fcb_req_wrapper_from_req<stat_struct>(req, true);               \
                                                                              \
        BOOST_SCOPE_EXIT_ALL(req, stat_struct_ptr) {                          \
          uv_fs_req_cleanup(req);                                             \
          if (stat_struct_ptr->fcb_ptr != nullptr) {  /* fstat case */        \
            thr_queue::default_par_queue().submit_work([=] {                  \
              stat_struct_ptr->fcb_ptr->decrement_counter();                  \
            });                                                               \
          }                                                                   \
        };                                                                    \
                                                                              \
        if (req->result < 0) {                                                \
    stat_struct_ptr->init_prom.set_exception(                                 \
        file_ ## OP ## _failure(req->result, "uv_fs_" #OP));                  \
          return;                                                             \
        } else {                                                              \
          stat_struct_ptr->init_prom.set_value({req->statbuf});               \
          return;                                                             \
        }                                                                     \
      };                                                                      \
                                                                              \
      uv_fs_ ## OP(uv_default_loop(), &stat_struct_ptr->req, EXTRA_ARGS,      \
          stat_cb);                                                           \
    };                                                                        \
    return thr_queue::event::uv_thr_cor_do<stat_result>(std::move(uv_code));  \
  })

aio_operation<stat_result>
stat(path p)
{
  return make_aio_operation([p = std::move(p)] () mutable {
    auto uv_code = [p = std::move(p)](auto prom) mutable {
  STAT_COMMON(nullptr, stat, p.c_str());
}

aio_operation<stat_result>
fstat(file &f)
{
  return make_aio_operation([cblock = f.cblock] () mutable {
    if (!cblock->increment_counter()) {
      auto excpt = file_truncate_failure(-EBADF, "fstat: already closed");
      return thr_queue::event::future_with_exception<uv_stat_t>(std::move(excpt));
    }
    auto uv_code = [cblock = std::move(cblock)](auto prom) mutable {
  STAT_COMMON(cblock, fstat, stat_struct_ptr->fcb_ptr->fd);
}

aio_operation<stat_result>
lstat(path p)
{
  return make_aio_operation([p = std::move(p)] () mutable {
    auto uv_code = [p = std::move(p)](auto prom) mutable {
  STAT_COMMON(nullptr, lstat, p.c_str());
}
}
}
