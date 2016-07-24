#include <cassert>
#include "stack_allocator_linux.h"
#include <sys/mman.h>
#include <stdexcept>
#include <logging/log.h>
#include <cstring>

namespace game_engine {
namespace thr_queue {
allocated_stack::allocated_stack()
{
  sc.size = 8 * 1024 * 1024;
  bottom_of_stack = (uint8_t *) mmap(nullptr, sc.size, PROT_READ | PROT_WRITE,
                                     MAP_PRIVATE | MAP_ANONYMOUS | MAP_GROWSDOWN
                                     | MAP_STACK, -1, 0);
  if (bottom_of_stack == MAP_FAILED) {
    LOG() << "mmap failed:" << strerror(errno);
    throw std::system_error(errno, std::system_category());
  }
  sc.sp = bottom_of_stack + sc.size;
}

allocated_stack::allocated_stack(empty_stack)
  : bottom_of_stack(nullptr)
{
  sc.sp   = nullptr;
  sc.size = 0;
}

allocated_stack::~allocated_stack()
{
  if (!bottom_of_stack) {
    return;
  }
  auto ret = munmap(bottom_of_stack, sc.size);
  if (ret != 0) {
    throw std::system_error(errno, std::system_category());
    LOG() << "munmap failed: " << strerror(errno);
  }
}

allocated_stack::allocated_stack(allocated_stack &&other)
  : sc(other.sc)
{
  other.sc.sp   = nullptr;
  other.sc.size = 0;
}

allocated_stack &allocated_stack::operator=(allocated_stack rhs)
{
  sc = rhs.sc;
  rhs.sc.sp   = 0;
  rhs.sc.size = 0;
  return *this;
}
}
}
