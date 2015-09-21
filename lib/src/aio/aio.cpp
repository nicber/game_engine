#include <aio/aio.h>

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
}
}
