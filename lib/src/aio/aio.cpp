#include <aio/aio.h>

namespace game_engine {
namespace aio {
aio_buffer::aio_buffer(size_t len)
  :len(len)
  , buf(new char[len])
  , own(true)
{}


aio_buffer::aio_buffer(size_t len_, char * buf_)
  :len(len_)
  , buf(buf_)
  , own(false)
{}

aio_buffer::aio_buffer(aio_buffer && other)
{
  aio_buffer tmp;
  swap(tmp, *this);
  swap(*this, other);
}

aio_buffer & aio_buffer::operator=(aio_buffer rhs)
{
  swap(*this, rhs);
  return *this;
}

aio_buffer::~aio_buffer()
{
  if (own) {
    delete[] buf;
  }
}

void swap(aio_buffer & lhs, aio_buffer & rhs)
{
  using std::swap;
  swap(lhs.len, rhs.len);
  swap(lhs.buf, rhs.buf);
  swap(lhs.own, rhs.own);
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
