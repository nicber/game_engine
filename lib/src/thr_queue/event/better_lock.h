#pragma once

#include "thr_queue/thread_api.h"

namespace game_engine {
namespace thr_queue {
namespace event {
class better_lock
{
public:
  better_lock( boost::mutex& m, boost::defer_lock_t ) : mt( m )
  {
  }

  better_lock( boost::mutex& m ) : mt( m )
  {
    lock( );
  }

  better_lock( const better_lock& ) = delete;
  better_lock( better_lock&& )      = delete;

  better_lock& operator=( better_lock ) = delete;

  ~better_lock( )
  {
    if ( already_locking ) {
      unlock( );
    }
  }

  void lock( )
  {
    assert( !already_locking );
    mt.lock( );
    already_locking = true;
  }

  void unlock( )
  {
    // ensures that every thread that sees that the mutex is unlocked will also
    // see already_locking = false. This is not guaranteed by unique_lock.
    assert( already_locking );
    already_locking = false;
    mt.unlock( );
  }

  bool owns_lock( )
  {
    return already_locking;
  }

private:
  boost::mutex& mt;
  bool already_locking = false;
};
}
}
}
