#include <atomic>
#include <cassert>
#include <concurrentqueue.h>
#include "stack_allocator.h"
#include <Windows.h>

// see https://msdn.microsoft.com/en-us/library/windows/desktop/aa366803(v=vs.85).aspx

namespace game_engine {
namespace thr_queue {
static std::atomic<DWORD> dwPageSize = 0;
static std::atomic<size_t> max_stack_size_in_pages = 0;
static std::atomic<double> ewma_stack_size = 5;

allocated_stack::allocated_stack()
{
  if (dwPageSize == 0) {
    SYSTEM_INFO sys_info;
    GetSystemInfo(&sys_info);
    dwPageSize = sys_info.dwPageSize;
    max_stack_size_in_pages = 8 * 1024 * 1024 / dwPageSize;
  }

  while (max_stack_size_in_pages == 0);

  sc.size = max_stack_size_in_pages * dwPageSize;
  bottom_of_stack = (uint8_t*) VirtualAlloc(nullptr, sc.size, MEM_RESERVE, PAGE_NOACCESS);
  assert(bottom_of_stack);
  sc.sp = bottom_of_stack + sc.size;

  commit_pages();
}

allocated_stack::allocated_stack(empty_stack)
 :bottom_of_stack(nullptr)
{
  sc.sp = nullptr;
  sc.size = 0;
}

allocated_stack::~allocated_stack()
{
  deallocate();
}

allocated_stack::allocated_stack(allocated_stack &&other)
 :sc(other.sc)
{
  other.sc.sp = nullptr;
  other.sc.size = 0;
}

allocated_stack &
allocated_stack::operator=(allocated_stack rhs)
{
  using std::swap;
  swap(pages, rhs.pages);
  swap(last_release, rhs.last_release);
  swap(bottom_of_stack, rhs.bottom_of_stack);
  swap(initially_commited_pages, rhs.initially_commited_pages);
  sc = rhs.sc;
  rhs.sc.sp = 0;
  rhs.sc.size = 0;
  return *this;
}

int
allocated_stack::filter_except_add_page(_EXCEPTION_POINTERS * ep)
{
  if (ep->ExceptionRecord->ExceptionCode != EXCEPTION_ACCESS_VIOLATION) {
    return EXCEPTION_CONTINUE_SEARCH;
  }
  auto op = ep->ExceptionRecord->ExceptionInformation[0];
  auto vaddress = ep->ExceptionRecord->ExceptionInformation[1];
  if (op != 0 && op != 1) {
    return EXCEPTION_CONTINUE_SEARCH;
  }
}

void
allocated_stack::release()
{
  const double a = 0.94;
  double ewma;
  double new_ewma;
  do {
    ewma = ewma_stack_size;
    new_ewma = (double)pages * (1 - a) + a*ewma;
  } while (!ewma_stack_size.compare_exchange_weak(ewma, new_ewma));
  last_release = std::chrono::system_clock::now();
}

void 
allocated_stack::deallocate()
{
  if (!bottom_of_stack) {
    return;
  }
  auto ret = VirtualFree(bottom_of_stack, 0, MEM_RELEASE);
  assert(ret);
}

void 
allocated_stack::commit_pages()
{
  if (!bottom_of_stack) {
    throw std::logic_error("trying to commit pages of an empty stack"
                           " (maybe a master coroutine?)");
  }
  initially_commited_pages = std::ceil(ewma_stack_size);
  initially_commited_pages = std::max((uint8_t) 1, initially_commited_pages);
  auto commit_bytes = initially_commited_pages * dwPageSize;
  auto ret = VirtualAlloc((uint8_t*)sc.sp - commit_bytes, commit_bytes,
                          MEM_COMMIT, PAGE_READWRITE);
  assert(ret);
}

allocated_stack
allocate_stack()
{
  return allocated_stack();
}

void
release_stack(allocated_stack)
{

}
}
}
