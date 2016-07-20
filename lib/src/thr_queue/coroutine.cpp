#include <boost/context/all.hpp>
#include <boost/lexical_cast.hpp>
#include <cassert>
#include "thr_queue/coroutine.h"
#include "global_thr_pool_impl.h"
#include <set>
#include "stack_allocator.h"

namespace game_engine {
namespace thr_queue {
static boost::mutex cor_mt;
static std::set<cor_data*> cor_set;

using exec_ctx = boost::context::execution_context<functor_ptr, allocated_stack*>;
// We might came back from another coroutine (think about a triangle, for example)
static thread_local exec_ctx *last_jump_from;


static bool coroutine_debug() 
{
  static bool res = [] {
    auto ptr = getenv("COROUTINE_DEBUG");
    int parse_res;
    bool res = false;
    if (ptr && boost::conversion::try_lexical_convert(ptr, parse_res)) {
      if (parse_res == 1) {
        res = true;
        LOG() << "enabling coroutine debugging: COROUTINE_DEBUG=" << 1;
      } else {
        res = false;
        LOG() << "disabling coroutine debugging: COROUTINE_DEBUG=" << parse_res;
      }
    } else if (ptr != nullptr) {
      LOG() << "disabling coroutine debugging: COROUTINE_DEBUG=" << ptr;
    } else {
      LOG() << "disabling coroutine debugging: COROUTINE_DEBUG is empty";
    }
    return res;
  } ();
  return res;
}

struct cor_data {
  cor_data()
   :alloc_stc(allocated_stack::empty_stack())
  {} 

  cor_data(functor_ptr func)
   :alloc_stc(allocate_stack())
  {
    if (coroutine_debug()) {
      boost::lock_guard<boost::mutex> l(cor_mt);
      auto ret = cor_set.insert(this);
      assert(ret.second);
    }

    auto start_func = [](exec_ctx came_from, functor_ptr f, allocated_stack *stc_ptr) {
      auto actual_start = came_from(nullptr, nullptr);
      assert(std::get<1>(actual_start) == nullptr);
      *last_jump_from = std::move(std::get<0>(actual_start));
      last_jump_from = nullptr;
#ifndef _WIN32
      (*f)();
#else
      auto func = [f_ptr = f.get(), stc_ptr] {
        __try {
          (*f_ptr)();
        } __except (stc_ptr->filter_except_add_page(GetExceptionInformation())) {
          abort();
        }
      };
      func();
#endif
      assert(master_coroutine->data_ptr->ctx);
      return std::move(master_coroutine->data_ptr->ctx);
    };

    boost::context::preallocated palloc(alloc_stc.sc.sp, alloc_stc.sc.size,
                                        alloc_stc.sc);
    ctx = exec_ctx(std::allocator_arg, palloc, donothing_allocator(),
                   start_func);
    // we pass the function it has to run and then we come back.
    auto new_ctx = std::get<0>(ctx(std::move(func), &alloc_stc));
    ctx = std::move(new_ctx);
  }

  ~cor_data() {
    if (coroutine_debug()) {
      boost::lock_guard<boost::mutex> l(cor_mt);
      auto ret = cor_set.erase(this);
      assert(ret == 1);
    }
    auto exectx = std::move(ctx);
    if (exectx) {
      LOG() << "destroying coroutine that has not finished yet";
    }
  }

  cor_data(const cor_data &) = delete;
  cor_data(cor_data &&) = delete;
  
  cor_data &operator=(cor_data) = delete;

  exec_ctx ctx;
  allocated_stack alloc_stc;

  coroutine_type typ;
  worker_thread *bound_thread = nullptr;

  //used in the linux implementation of blocking aio operations.
  //see aio_operation_t<T>::perform() for further information.
  worker_thread *forbidden_thread = nullptr;
};

coroutine_type coroutine::type() const { return data_ptr->typ; }

void coroutine::switch_to_from(coroutine &other) {
#ifndef _WIN32
  if (cor_data->bound_thread) {
    assert(cor_data->bound_thread == this_wthread);
  } else {
    cor_data->bound_thread = this_wthread;
  }
#endif
  assert(last_jump_from == nullptr);
  last_jump_from = &other.data_ptr->ctx;
  auto go_back = data_ptr->ctx(nullptr, nullptr);
  if (last_jump_from) {
    *last_jump_from = std::move(std::get<0>(go_back));
    last_jump_from = nullptr;
  }
}

void
coroutine::set_forbidden_thread(worker_thread *thr)
{
  data_ptr->forbidden_thread = thr;
}

bool
coroutine::can_be_run_by_thread(worker_thread *thr) const
{
  return thr != data_ptr->forbidden_thread;
}

std::intptr_t
coroutine::get_id() const noexcept
{
  return (std::intptr_t) data_ptr.get();
}

coroutine::coroutine()
 :data_ptr(new cor_data)
{}

coroutine::~coroutine()
{} 

coroutine::coroutine(functor_ptr func)
 :data_ptr(new cor_data(std::move(func)))
{}

void
cor_data_deleter::operator()(cor_data *ptr)
{
  delete ptr;
}
}
}
