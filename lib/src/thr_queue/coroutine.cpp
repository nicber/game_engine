#include "thr_queue/coroutine.h"
#include <cassert>

namespace game_engine {
namespace thr_queue {

thread_local coroutine *current_coroutine = nullptr;

coroutine::coroutine(coroutine &&other) : coroutine(coroutine_type::master) {
  swap(*this, other);
}

coroutine &coroutine::operator=(coroutine &&rhs) {
  using std::swap;
  coroutine temp(
      coroutine_type::master); // make sure we don't allocate a stack.

  swap(temp, rhs);
  swap(temp, *this);

  return *this;
}

coroutine::~coroutine() {
  if (typ == coroutine_type::master) {
    assert(!stack &&
           "master coroutines don't have allocated stacks since the OS "
           "takes care of that");
    delete ctx;
  } else {
    assert(stack &&
           "if this is not a master_coroutine then it should have a stack");
  }
}

std::chrono::high_resolution_clock::time_point
coroutine::creation_time() const {
  return creation_tim;
}

coroutine_type coroutine::type() const { return typ; }

void coroutine::switch_to() {
  auto func_ptr = reinterpret_cast<intptr_t>(function.get());
  boost::context::jump_fcontext(current_coroutine->ctx, ctx, func_ptr);
}

coroutine::coroutine()
    : creation_tim(std::chrono::high_resolution_clock::now()) {}

coroutine::coroutine(coroutine_type cor_typ) : coroutine() {
  ctx = new boost::context::fcontext_t();
}

size_t coroutine::default_stacksize() {
  return 1024 * 1024; // 1 MB
}

void coroutine::make_current_coroutine() {
  if (typ != coroutine_type::master) {
    throw std::logic_error(
        "non-master coroutines can't be made current explicitly");
  }

  current_coroutine = this;
}

void swap(coroutine &lhs, coroutine &rhs) {
  using std::swap;
  swap(lhs.ctx, rhs.ctx);
  swap(lhs.stack, rhs.stack);
  swap(lhs.function, rhs.function);
  swap(lhs.typ, rhs.typ);
  swap(lhs.creation_tim, rhs.creation_tim);
}
}
}
