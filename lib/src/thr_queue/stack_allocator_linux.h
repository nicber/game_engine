#pragma once

#include <boost/context/stack_context.hpp>
#include <stdint.h>

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

private:
  allocated_stack();

  friend allocated_stack allocate_stack();

  friend void release_stack(allocated_stack);

  friend struct cor_data;

private:
  boost::context::stack_context         sc;
  uint8_t *bottom_of_stack = nullptr;
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
