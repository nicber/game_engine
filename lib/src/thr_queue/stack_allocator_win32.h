#pragma once

#include <boost/context/stack_context.hpp>
#include "cor_data.h"

namespace game_engine {
namespace thr_queue {
class allocated_stack {
public:
  struct empty_stack {
  };

  allocated_stack(empty_stack);

  ~allocated_stack();

  allocated_stack(const allocated_stack &) = delete;

  allocated_stack(allocated_stack &&other);

  allocated_stack &operator=(allocated_stack);

  int filter_except_add_page(_EXCEPTION_POINTERS *ep);

private:
  allocated_stack();

  void deallocate();

  void commit_pages();

  void release();

  friend allocated_stack allocate_stack();

  friend void release_stack(allocated_stack);

  friend struct cor_data;

private:
  size_t                                pages                    = 0;
  size_t                                initially_commited_pages = 0;
  std::chrono::system_clock::time_point last_release;
  boost::context::stack_context         sc;
  uint8_t                               *bottom_of_stack;
};

allocated_stack allocate_stack();

void release_stack(allocated_stack stack);

class donothing_allocator {
public:
  void deallocate(boost::context::stack_context &)
  {
  }
};
}
}
