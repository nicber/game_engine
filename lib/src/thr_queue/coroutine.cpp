#include <cassert>
#include "thr_queue/coroutine.h"
#include "global_thr_pool_impl.h"
#include <set>

static boost::mutex cor_mt;
static std::set<game_engine::thr_queue::coroutine::stackctx*> cor_set;

namespace game_engine {
namespace thr_queue {

struct coroutine::stackctx {
  boost::context::fcontext_t ctx;
  char stack[64*1024]; //64kb

  stackctx() {
    boost::lock_guard<boost::mutex> l(cor_mt);
    auto ret = cor_set.insert(this);
    assert(ret.second);
  }

  ~stackctx() {
    boost::lock_guard<boost::mutex> l(cor_mt);
    auto ret = cor_set.erase(this);
    assert(ret == 1);
  }
};

coroutine::coroutine(coroutine &&other) noexcept : coroutine() {
  swap(*this, other);
}

coroutine &coroutine::operator=(coroutine &&rhs) noexcept {
  using std::swap;
  coroutine temp; // make sure we don't allocate a stack.

  swap(temp, rhs);
  swap(temp, *this);

  return *this;
}

coroutine::~coroutine() {
  if (typ == coroutine_type::master) {
    // master coroutines don't have allocated stacks since the OS 
    // takes care of that.
  } else {
    assert(stack_and_ctx &&
           "if this is not a master_coroutine then it should have a stack");
    stack_and_ctx.reset(nullptr);
  }
}

coroutine_type coroutine::type() const { return typ; }

void coroutine::switch_to_from(coroutine &other) {
#ifndef _WIN32
  if (bound_thread) {
    assert(bound_thread == this_wthread);
  } else {
    bound_thread = this_wthread;
  }
#endif
  auto func_ptr = reinterpret_cast<intptr_t>(function.get());
  auto &this_ctx  = typ == coroutine_type::master ? ctx : stack_and_ctx->ctx;
  auto &other_ctx = other.typ == coroutine_type::master ? other.ctx
                                                        : other.stack_and_ctx->ctx;
  boost::context::jump_fcontext(&other_ctx, this_ctx, func_ptr, true);
}

std::intptr_t
coroutine::get_id() const noexcept
{
  if (typ == coroutine_type::master) {
    return (std::intptr_t) ctx;
  } else {
    return (std::intptr_t) stack_and_ctx->ctx;
  }
}

coroutine::coroutine() :
  typ(coroutine_type::master)
{}

coroutine::coroutine(std::unique_ptr<functor> func)
 :stack_and_ctx(std::make_unique<stackctx>())
 ,function(std::move(func))
 ,typ(coroutine_type::user)
{
  auto stack_size = sizeof(stackctx::stack);
  auto stack_start = (char*)&stack_and_ctx->stack + stack_size;
  auto start_func = [](intptr_t ptr) {
    auto *reint_ptr = reinterpret_cast<functor *>(ptr);
    (*reint_ptr)();
    finish_coroutine();
  };
  stack_and_ctx->ctx = boost::context::make_fcontext(stack_start, stack_size,
                                                     start_func);
}

void coroutine::finish_coroutine() { global_thr_pool.yield(); }

void swap(coroutine &lhs, coroutine &rhs) {
  using std::swap;
  if (lhs.typ == rhs.typ) {
    if (lhs.typ == coroutine_type::master) {
      swap(lhs.ctx, rhs.ctx);
    } else {
      swap(lhs.stack_and_ctx, rhs.stack_and_ctx);
    }
  } else if (lhs.typ == coroutine_type::master) {
    auto lhs_ctx = std::move(lhs.ctx);
    new (&lhs.stack_and_ctx) decltype(lhs.stack_and_ctx)(std::move(rhs.stack_and_ctx));
    rhs.ctx = std::move(lhs_ctx);
  } else {
    auto rhs_ctx = std::move(rhs.ctx);
    new (&rhs.stack_and_ctx) decltype(rhs.stack_and_ctx)(std::move(lhs.stack_and_ctx));
    lhs.ctx = std::move(rhs_ctx);
  }
  swap(lhs.function, rhs.function);
  swap(lhs.typ, rhs.typ);
  swap(lhs.bound_thread, rhs.bound_thread);
}
}
}
