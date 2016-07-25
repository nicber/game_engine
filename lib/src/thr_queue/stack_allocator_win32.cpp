#include <atomic>
#include <boost/context/stack_traits.hpp>
#include <cassert>
#include <concurrentqueue.h>
#include <logging/log.h>
#include "stack_allocator.h"
#include <Windows.h>

// see https://msdn.microsoft.com/en-us/library/windows/desktop/aa366803(v=vs.85).aspx

namespace game_engine {
namespace thr_queue {
std::atomic<double> ewma_stack_size{1};

allocated_stack::allocated_stack()
{
  sc.size = stack_size;
  bottom_of_stack = (uint8_t *) VirtualAlloc(nullptr, sc.size, MEM_RESERVE, PAGE_READWRITE);
  assert(bottom_of_stack);
  sc.sp = bottom_of_stack + sc.size;

  if (!bottom_of_stack) {
    throw std::logic_error("trying to commit pages of an empty stack"
                             " (maybe a master coroutine?)");
  }
  initially_commited_pages = std::ceil(ewma_stack_size);
  initially_commited_pages = std::max(stack_props.min_stack_size_in_pages, initially_commited_pages);
  auto commit_bytes = initially_commited_pages * stack_props;
  auto ret          = VirtualAlloc((uint8_t *) sc.sp - commit_bytes, commit_bytes, MEM_COMMIT, PAGE_READWRITE);
  assert(ret);
  DWORD old_prot;
  bool  ret2        = VirtualProtect((uint8_t *) sc.sp - commit_bytes, 1, PAGE_READWRITE | PAGE_GUARD, &old_prot);
}

allocated_stack::~allocated_stack()
{
  if (!bottom_of_stack) {
    return;
  }
  auto ret = VirtualFree(bottom_of_stack, 0, MEM_RELEASE);
  assert(ret);
}

int platform::SEH_filter_except_add_page(_EXCEPTION_POINTERS *ep)
{
  if (ep->ExceptionRecord->ExceptionCode != EXCEPTION_ACCESS_VIOLATION) {
    return EXCEPTION_CONTINUE_SEARCH;
  }
  auto op       = ep->ExceptionRecord->ExceptionInformation[0];
  auto vaddress = (uint8_t *) ep->ExceptionRecord->ExceptionInformation[1];
  if (op != 0 && op != 1) {
    return EXCEPTION_CONTINUE_SEARCH;
  }
  //leave a guard page
  if (bottom_of_stack + dwPageSize < vaddress && vaddress < sc.sp) {
    auto bottom_of_page = (uint8_t *) ((std::intptr_t) vaddress / stack_props.page_size * stack_props.page_size);
    auto numbytes       = (std::intptr_t) sc.sp - (std::intptr_t) bottom_of_page;
    assert(numbytes % dwPageSize == 0);
    auto ret = VirtualAlloc(bottom_of_page, (size_t) numbytes, MEM_COMMIT, PAGE_READWRITE);
    if (!ret) {
      return EXCEPTION_CONTINUE_SEARCH;
    } else {
      pages = numbytes / dwPageSize;
      return EXCEPTION_CONTINUE_EXECUTION;
    }
  } else {
    return EXCEPTION_CONTINUE_SEARCH;
  }
}

void platform::allocated_stack::plat_release()
{
  if (!bottom_of_stack) {
    return;
  }
  if (pages_were_committed) {
    return;
  }
  const double a = 0.94;
  double       ewma;
  double       new_ewma;
  do {
    ewma     = ewma_stack_size;
    new_ewma = (double) pages * (1 - a) + a * ewma;
  } while (!ewma_stack_size.compare_exchange_weak(ewma, new_ewma));
  LOG() << "New ewma: " << new_ewma;
}

static boost::mutex commit_pages_mt;

bool platform::allocated_stack::commit_pages_prob()
{
  boost::lock_guard<boost::mutex>    l(commit_pages_mt);
  static std::mt19937                mt{std::random_device().operator()()};
  static std::bernoulli_distribution d(0.01);
  return d(mt);
}

double platform::allocated_stack::committed_pages()
{
  return pages_were_committed ? (double) pages * 1.2 : (double) pages;
}
}
}
