#pragma once

#include "cor_data.h"
#include "stack_allocator.h"
#include "thr_queue/coroutine.h"
#include <boost/context/execution_context.hpp>

namespace game_engine {
namespace thr_queue {
using exec_ctx = boost::context::execution_context< functor_ptr, allocated_stack* >;

// We might came back from another coroutine (think about a triangle, for example)
extern thread_local exec_ctx* last_jump_from;

struct cor_data
{
  cor_data( );

  ~cor_data( );

  cor_data( const cor_data& ) = delete;

  cor_data( cor_data&& ) = delete;

  cor_data( functor_ptr func );

  cor_data& operator=( cor_data ) = delete;

  exec_ctx ctx;
  game_engine::thr_queue::allocated_stack alloc_stc;

  game_engine::thr_queue::coroutine_type typ;
  game_engine::thr_queue::worker_thread* bound_thread = nullptr;

  // used in the linux implementation of blocking aio operations.
  // see aio_operation_t<T>::perform() for further information.
  game_engine::thr_queue::worker_thread* forbidden_thread = nullptr;
};
}
}
