#include "cor_data.h"
#include "global_thr_pool_impl.h"

namespace game_engine {
namespace thr_queue {
coroutine_type
coroutine::type( ) const
{
  return data_ptr->typ;
}

void
coroutine::switch_to_from( coroutine& other )
{
#ifndef _WIN32
  if ( data_ptr->bound_thread ) {
    assert( data_ptr->bound_thread == this_wthread );
  } else {
    data_ptr->bound_thread = this_wthread;
  }
#endif
  assert( last_jump_from == nullptr );
  last_jump_from = &other.data_ptr->ctx;
  auto go_back   = data_ptr->ctx( nullptr, nullptr );
  if ( last_jump_from ) {
    *last_jump_from = std::move( std::get< 0 >( go_back ) );
    last_jump_from  = nullptr;
  }
}

void
coroutine::set_forbidden_thread( worker_thread* thr )
{
  data_ptr->forbidden_thread = thr;
}

bool
coroutine::can_be_run_by_thread( worker_thread* thr ) const
{
  return thr != data_ptr->forbidden_thread;
}

std::intptr_t
coroutine::get_id( ) const noexcept
{
  return (std::intptr_t) data_ptr.get( );
}

coroutine::coroutine( ) : data_ptr( new cor_data )
{
}

coroutine::~coroutine( )
{
}

coroutine::coroutine( functor_ptr func ) : data_ptr( new cor_data( std::move( func ) ) )
{
}

void
cor_data_deleter::operator( )( cor_data* ptr )
{
  delete ptr;
}
}
}
