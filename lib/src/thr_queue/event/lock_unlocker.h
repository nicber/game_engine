#pragma once

#include "thr_queue/thread_api.h"

namespace game_engine {
namespace thr_queue {
namespace event {

template < typename L >
class lock_unlocker
{
public:
  lock_unlocker( L& l ) : lock( l )
  {
  }

  ~lock_unlocker( )
  {
    assert( lock.owns_lock( ) );
    lock.unlock( );
  }

private:
  L& lock;
};
}
}
}
