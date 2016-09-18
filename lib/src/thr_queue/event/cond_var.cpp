#include "thr_queue/event/cond_var.h"
#include "../global_thr_pool_impl.h"
#include "better_lock.h"
#include "lock_unlocker.h"
#include "thr_queue/coroutine.h"

namespace game_engine {
namespace thr_queue {
namespace event {
void
condition_variable::wait( boost::unique_lock< mutex >& lock )
{
  assert( lock.owns_lock( ) );
  boost::unique_lock< mutex > local_lock( *lock.release( ), boost::adopt_lock );
  better_lock lock_std( mt );
  global_thr_pool.yield( [&]( coroutine running ) {
    lock_unlocker< better_lock > l_unlock_std_mt( lock_std );
    lock_unlocker< boost::unique_lock< mutex > > l_unlock_co_mt( local_lock );

    waiting_cors.emplace_back( std::move( running ) );
  } );

  // If this coroutine is resumed by a thread different from the one that
  // yielded,
  // then it is possible that the changes to internal state of the 'lock'
  // variable will
  // not have been yet acknowledged by the resuming thread.
  // This special case will lead to a difficult to debug race condition
  // involving
  // the owns_lock member variable.
  // This is why we use local_lock.
  assert( !local_lock.owns_lock( ) );
  lock = boost::unique_lock< mutex >( *local_lock.release( ) );
}

void
condition_variable::notify( )
{
  boost::unique_lock< boost::mutex > mt_lock( mt );
  decltype( waiting_cors ) wc;
  swap( wc, waiting_cors );
  mt_lock.unlock( );

  auto begin_move = std::make_move_iterator( wc.begin( ) );
  auto end_move   = std::make_move_iterator( wc.end( ) );
  global_thr_pool.schedule( begin_move, end_move, true );
}

condition_variable::~condition_variable( )
{
  boost::lock_guard< boost::mutex > mt_lock( mt );
  assert( waiting_cors.size( ) == 0 && "coroutines can't be destroyed by the condition variable." );
}
}
}
}
