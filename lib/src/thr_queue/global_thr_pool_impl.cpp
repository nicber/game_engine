#include "global_thr_pool_impl.h"

#include <algorithm>
#define BOOST_SCOPE_EXIT_CONFIG_USE_LAMBDAS
#include <boost/scope_exit.hpp>
#include <iterator>
#include <logging/log.h>

namespace game_engine {
namespace thr_queue {
worker_thread_internals::worker_thread_internals( generic_work_data& dat )
  : stopped( false )
  , ctok( dat.work_queue )
  , ptok( dat.work_queue )
  , ctok_prio( dat.work_queue_prio )
  , ptok_prio( dat.work_queue_prio )
{
}

void
generic_worker_thread::start_thread( )
{
  internals.emplace( get_data( ) );
  internals->thr = boost::thread( [this] { loop( ); } );
}

void
generic_worker_thread::schedule_coroutine( coroutine cor )
{
  LOG( ) << "Scheduled cor: " << cor.get_id( );
#ifdef _WIN32
  abort( );
#else
  ++get_internals( ).thread_queue_size;
  get_internals( ).thread_queue.enqueue( std::move( cor ) );
  wakeup( );
#endif
}

void
generic_worker_thread::do_work( )
{
  bool could_work;
  int number_units_of_work   = 0;
  bool only_run_thread_queue = false;
  do {
    could_work = false;

    coroutine work_to_do;

    if ( run_next ) {
      work_to_do = std::move( run_next.get( ) );
      run_next   = boost::none;
      goto do_work;
    }

    while ( get_internals( ).thread_queue_size > 0 ) {
      if ( get_internals( ).thread_queue.try_dequeue( work_to_do ) ) {
        --get_internals( ).thread_queue_size;
        goto do_work;
      }
    }

    while ( get_data( ).work_queue_prio_size > 0 && !only_run_thread_queue ) {
      if ( get_data( ).work_queue_prio.try_dequeue_from_producer( internals->ptok_prio, work_to_do ) ||
           get_data( ).work_queue_prio.try_dequeue( work_to_do ) ) {
        --get_data( ).work_queue_prio_size;
        goto do_work;
      }
    }

    while ( get_data( ).work_queue_size > 0 && !only_run_thread_queue ) {
      if ( get_data( ).work_queue.try_dequeue_from_producer( internals->ptok, work_to_do ) ||
           get_data( ).work_queue.try_dequeue( work_to_do ) ) {
        --get_data( ).work_queue_size;
        goto do_work;
      }
      if ( get_data( ).work_queue_prio.try_dequeue_from_producer( internals->ptok_prio, work_to_do ) ||
           get_data( ).work_queue_prio.try_dequeue( work_to_do ) ) {
        --get_data( ).work_queue_prio_size;
        goto do_work;
      }
    }

    could_work = false;
    break;

  do_work:
    ++get_data( ).working_threads;
    if ( !work_to_do.can_be_run_by_thread( this_wthread ) ) {
      // we reschedule it and hope it is run by a different thread.
      LOG( ) << "Rescheduling cor: " << work_to_do.get_id( ) << " thr id: " << boost::this_thread::get_id( );
      global_thr_pool.schedule( std::move( work_to_do ), true );
      only_run_thread_queue = true;
      continue;
    }
    ++number_units_of_work;
    could_work        = true;
    running_coroutine = &work_to_do;

    work_to_do.set_forbidden_thread( nullptr );

    work_to_do.switch_to_from( *master_coroutine );
    assert( running_coroutine == &work_to_do );
    if ( *after_yield ) {
      ( *after_yield )( std::move( work_to_do ) );
      *after_yield = nullptr;
    }
    running_coroutine = master_coroutine;

    // if we think that other threads are waiting apart
    // from this one, we wake them up.
    global_thr_pool.plat_wakeup_threads( );

    BOOST_SCOPE_EXIT_ALL( & )
    {
      --get_data( ).working_threads;
    };
  } while ( could_work );
  LOG( ) << "Thread " << boost::this_thread::get_id( ) << " performed " << number_units_of_work;
}

worker_thread_internals&
generic_worker_thread::get_internals( )
{
  assert( internals.is_initialized( ) );
  return *internals;
}

worker_thread::worker_thread( work_data_combined& dat ) : platform::worker_thread_impl( dat )
{
  start_thread( );
}

worker_thread::~worker_thread( )
{
  if ( get_internals( ).thr.joinable( ) ) {
    LOG( ) << "WARNING: Joining worker thread in its destructor.";
    get_internals( ).thr.join( );
  }
  assert( get_internals( ).stopped );
}

global_thread_pool::global_thread_pool( )
  : hardware_concurrency( std::max( 1u, boost::thread::hardware_concurrency( ) ) )
  , work_data( hardware_concurrency )
{
  auto c_threads = hardware_concurrency + 8;

  boost::lock_guard< boost::mutex > lock( threads_mt );
  for ( size_t i = 0; i < c_threads; ++i ) {
    threads.emplace_back( work_data );
  }
}

global_thread_pool::~global_thread_pool( )
{
  boost::unique_lock< boost::mutex > l( threads_mt );
  work_data.shutting_down = true;
  for ( auto it = threads.begin( ); it != threads.end( ); ) {
    if ( it->get_internals( ).stopped ) {
      it->get_internals( ).thr.join( );
      it = threads.erase( it );
      continue;
    }
    it->wakeup( );
    ++it;
  }
  while ( threads.size( ) ) {
    auto& thr = threads.back( );
    l.unlock( );
    thr.get_internals( ).thr.join( );
    l.lock( );
    threads.pop_back( );
  }
  assert( work_data.working_threads == 0 );
  assert( work_data.number_threads == 0 );
}

void
global_thread_pool::schedule( coroutine cor, bool first )
{
  auto move_iter = std::make_move_iterator( &cor );
  schedule( move_iter, move_iter + 1, first );
}

void
global_thread_pool::yield( )
{
  assert( running_coroutine != nullptr );
  assert( running_coroutine != master_coroutine && "we can't yield from the master_coroutine" );
  assert( master_coroutine->data_ptr->ctx );
  master_coroutine->switch_to_from( *running_coroutine );
}

void
global_thread_pool::yield( after_yield_f func )
{
  *after_yield = std::move( func );
  yield( );
}

void
global_thread_pool::yield_to( coroutine next, after_yield_f after_yield )
{
  assert( !run_next );
  run_next = std::move( next );
  yield( std::move( after_yield ) );
}

thread_local global_thread_pool::after_yield_f* after_yield = nullptr;
thread_local coroutine* master_coroutine                    = nullptr;
thread_local coroutine* running_coroutine                   = nullptr;
thread_local boost::optional< coroutine > run_next;
thread_local worker_thread* this_wthread = nullptr;
global_thread_pool global_thr_pool;
}
}
