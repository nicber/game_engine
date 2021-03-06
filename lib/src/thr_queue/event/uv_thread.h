#pragma once

#include <atomic>
#include <deque>
#include <memory>
#include <thr_queue/event/future.h>
#include <thr_queue/functor.h>
#include <thr_queue/thread_api.h>
#include <uv.h>

namespace game_engine {
namespace thr_queue {
namespace event {
struct uv_thread
{
  uv_thread( );

  ~uv_thread( );

  std::atomic< bool > should_stop;
  std::atomic< bool > async_constructed{ false };
  std::unique_ptr< uv_async_t > global_async = nullptr;
  std::deque< std::unique_ptr< functor > > init_start_requests;
  boost::mutex init_start_requests_mt;
  boost::thread thr;
};

uv_thread& get_uv_thr( );

/** \brief Schedules the passed function to run in the libuv thread and then
 * returns a boost::future that will be signaled when the function will be
 * executed.
 */
template < typename F >
boost::future< void > uv_thr_sync_do( F func );

/** \brief Schedules to passed function to be run on the libuv thread.
 * The function will be passed a promise<Ret> that it can use to indicate
 * progress to another piece of code.
 * The corresponding future is returned by this function.
 */
template < typename Ret, typename F >
event::future< Ret > uv_thr_cor_do( F func );

/** \brief Initializes a uv_async_t structure synchronously.
 */
void uv_thr_async_init( uv_async_t* async, uv_async_cb f_ptr );
}
}
}

#include "uv_thread.inl"
