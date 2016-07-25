#pragma once

#include <atomic>
#include <boost/context/stack_context.hpp>
#include <boost/context/stack_traits.hpp>
#include <chrono>

#ifndef _WIN32
#include "stack_allocator_linux.h"
#else
#include "stack_allocator_win32.h"
#endif

namespace game_engine {
namespace thr_queue {
struct stack_props_t {
  static constexpr size_t stack_size = 8 * 1024 * 1024;
  size_t                  page_size;
  size_t                  max_stack_size_in_pages;
  size_t                  min_stack_size_in_pages;

  stack_props_t()
    : page_size(boost::context::stack_traits::page_size())
    , max_stack_size_in_pages(stack_size / page_size)
    , min_stack_size_in_pages(boost::context::stack_traits::minimum_size() / page_size)
  {
  }
};

extern const stack_props_t stack_props;

class allocated_stack : protected platform::allocated_stack {
public:
  struct empty_stack {
  };

  allocated_stack();

  ~allocated_stack();

  allocated_stack(const allocated_stack &) = delete;

  allocated_stack(allocated_stack &&other);

  allocated_stack &operator=(allocated_stack);

private:
  struct alloc{};
  allocated_stack(alloc);

  void release();

  friend allocated_stack allocate_stack();

  friend void release_stack(allocated_stack);

  friend struct cor_data;

private:
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
