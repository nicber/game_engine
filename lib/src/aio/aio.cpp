#include <aio/aio.h>
#include "../thr_queue/global_thr_pool_impl.h"

namespace game_engine {
namespace aio {
aio_buffer::aio_buffer(uv_buf_t &&buf)
 :uv_buf_t(buf)
{
  buf.base = nullptr;
  buf.len = 0;
}

aio_buffer::aio_buffer(size_type length)
 :uv_buf_t(uv_buf_init((char *)malloc(length), length))
{
}

aio_buffer::aio_buffer(aio_buffer && other)
 :aio_buffer()
{
  swap(*this, other);
}

aio_buffer & aio_buffer::operator=(aio_buffer rhs)
{
  swap(*this, rhs);
  return *this;
}

void
aio_buffer::append(aio_buffer other)
{
  char *new_base = (char *)realloc(base, len + other.len);
  if (!new_base) {
    throw std::bad_alloc();
  }
  memcpy(new_base + len, other.base, other.len);
  base = new_base;
  len += other.len;
}

uv_buf_t aio_buffer::get_subbuffer(size_type shift)
{
  if (shift >= len) {
    throw std::logic_error("cannot shift an amount larger than the buffer size");
  }

  auto new_length = len - shift;
  return uv_buf_init(base + shift, new_length);
}

aio_buffer::~aio_buffer()
{
  free(base);
}

aio_buffer::aio_buffer()
 :uv_buf_t(uv_buf_init(nullptr, 0))
{}

void swap(aio_buffer & lhs, aio_buffer & rhs)
{
  using std::swap;
  swap(lhs.base, rhs.base);
  swap(lhs.len, rhs.len);
}

aio_operation_base::~aio_operation_base()
{
  assert(!perform_on_destr || already_performed);
}

void aio_operation_base::set_perform_on_destruction(bool flag)
{
  perform_on_destr = flag;
}

bool aio_operation_base::get_perform_on_destruction() const
{
  return perform_on_destr;
}

void aio_operation_base::replace_running_cor_and_jump(perform_helper_base & helper, thr_queue::coroutine work_cor)
{
  helper.caller_coroutine = std::move(*thr_queue::running_coroutine_or_yielded_from);
  *thr_queue::running_coroutine_or_yielded_from = std::move(work_cor);
  helper.caller_coroutine->switch_to_from(*thr_queue::running_coroutine_or_yielded_from);
}


void perform_helper_base::about_to_block()
{
  if (caller_coroutine) {
    apc_pending_exec = true;
    QueueUserAPC([](ULONG_PTR ptr) {
      auto &that = *reinterpret_cast<perform_helper_base*>(ptr);
      if (!that.apc_pending_exec) {
        return;
      }
      that.apc_pending_exec = false;
      thr_queue::global_thr_pool.schedule(std::move(that.caller_coroutine.get()), true);
      that.caller_coroutine.reset();
    }, thr_queue::this_wthread->thr.native_handle(), PtrToUlong(this));
  }
}

void perform_helper_base::cant_block_anymore()
{
  if (apc_pending_exec) {
    apc_pending_exec = false;
    WaitForSingleObjectEx(INVALID_HANDLE_VALUE, 0, true);
  }
}

perform_helper_base::~perform_helper_base()
{
  cant_block_anymore();
  if (caller_coroutine) {
    thr_queue::coroutine this_cor = std::move(*thr_queue::running_coroutine_or_yielded_from);
    *thr_queue::running_coroutine_or_yielded_from = std::move(caller_coroutine.get());
  }
}

}
}
