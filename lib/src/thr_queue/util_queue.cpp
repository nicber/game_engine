#include "thr_queue/util_queue.h"
#include "global_thr_pool_impl.h"
#include "thr_queue/global_thr_pool.h"
#include <cassert>

namespace game_engine {
namespace thr_queue {
static void
sch_par_queue( queue& q )
{
  queue qw( steal_work, q );
  schedule_queue( std::move( qw ) );
}

static queue def_par_queue( queue_type::parallel, sch_par_queue );
queue&
default_par_queue( )
{
  return def_par_queue;
}

struct ser_queue_comm
{
  event::mutex mt_exec;
  boost::mutex mt_qu;
  queue qu       = queue( queue_type::serial );
  bool cor_sched = false;
};

queue
ser_queue( )
{
  auto comm = std::make_shared< ser_queue_comm >( );

  queue q( queue_type::serial, [comm = std::move( comm )]( queue & q ) {
    boost::lock_guard< boost::mutex > lock( comm->mt_qu );

    comm->qu.append_queue( { steal_work, q } );

    if ( comm->cor_sched ) {
      return;
    }

    queue q_ser( queue_type::serial );

    q_ser.submit_work( [comm]( ) mutable {
      boost::lock_guard< event::mutex > lock_exec( comm->mt_exec );
      boost::unique_lock< boost::mutex > lock( comm->mt_qu );

      assert( comm->cor_sched );

      comm->cor_sched = false;
      auto q_work     = std::move( comm->qu );
      comm->qu        = queue( queue_type::serial );
      lock.unlock( );

      q_work.run_until_empty( );
    } );

    schedule_queue( std::move( q_ser ) );
    comm->cor_sched = true;
  } );

  return q;
}
}
}
