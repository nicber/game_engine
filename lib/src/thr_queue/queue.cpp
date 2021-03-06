#include <atomic>
#include <iterator>

#include "thr_queue/global_thr_pool.h"
#include "thr_queue/queue.h"
#include "thr_queue/thread_api.h"

namespace game_engine {
namespace thr_queue {
void
swap( queue& lhs, queue& rhs )
{
  boost::lock( lhs.queue_mut, rhs.queue_mut );
  boost::lock_guard< boost::recursive_mutex > lock_lhs( lhs.queue_mut, boost::adopt_lock );
  boost::lock_guard< boost::recursive_mutex > lock_rhs( rhs.queue_mut, boost::adopt_lock );

  using std::swap;
  swap( lhs.work_queue, rhs.work_queue );
  swap( lhs.typ, rhs.typ );
  swap( lhs.cb_added, rhs.cb_added );
}

queue::queue( queue&& rhs ) : typ( queue_type::parallel )
{
  swap( *this, rhs );
}

queue&
queue::operator=( queue&& rhs )
{
  queue temp( queue_type::parallel );
  using std::swap;
  swap( temp, rhs );
  swap( *this, temp );
  return *this;
}

queue::queue( queue_type ty ) : typ( ty )
{
}

queue::queue( queue_type ty, queue::callback_t cb ) : typ( ty ), cb_added( std::move( cb ) )
{
}

queue::queue( steal_work_t, queue& other )
{
  boost::unique_lock< boost::recursive_mutex > lock( other.queue_mut );
  typ        = other.typ;
  work_queue = std::move( other.work_queue );
}

void
queue::append_queue( queue q )
{
  boost::lock( queue_mut, q.queue_mut );
  boost::lock_guard< boost::recursive_mutex > my_lock( queue_mut, boost::adopt_lock );
  boost::lock_guard< boost::recursive_mutex > q_lock( q.queue_mut, boost::adopt_lock );

  if ( typ == q.typ ) {
    std::move( q.work_queue.begin( ), q.work_queue.end( ), std::back_inserter( work_queue ) );

  } else if ( typ == queue_type::parallel && q.typ == queue_type::serial ) {
    auto func = [q = std::move( q )]( ) mutable
    {
      q.run_until_empty( );
    };

    work_queue.emplace_back( new spec_functor< decltype( func ) >( std::move( func ) ) );

  } else if ( typ == queue_type::serial && q.typ == queue_type::parallel ) {

    auto func = [q = std::move( q )]( ) mutable
    {
      event::mutex mt;
      event::condition_variable cv;
      boost::unique_lock< event::mutex > lock( mt );

      auto size = q.work_queue.size( );
      schedule_queue_first( std::move( q ), cv, mt, size );
      cv.wait( lock );
    };

    work_queue.emplace_back( new spec_functor< decltype( func ) >( std::move( func ) ) );
  }

  if ( cb_added ) {
    cb_added( *this );
  }
}

bool
queue::run_once( )
{
  boost::unique_lock< boost::recursive_mutex > lock( queue_mut );

  if ( work_queue.size( ) == 0 ) {
    return false;
  }
  auto work = std::move( work_queue.front( ) );
  work_queue.pop_front( );

  // we want to allow another thread to call this member function while we are
  // running the work unit.
  if ( typ == queue_type::parallel ) {
    lock.unlock( );
  }
  ( *work )( );
  return true;
}

void
queue::run_until_empty( )
{
  while ( work_queue.size( ) > 0 ) {
    auto work = std::move( work_queue.front( ) );
    work_queue.pop_front( );
    ( *work )( );
  }
}

queue_type
queue::type( ) const
{
  return typ;
}
}
}
