#include <aio/aio.h>

namespace game_engine {
namespace aio {
aio_result_t::aio_result_t()
 :finished(false)
 ,read_bytes(0)
{}

bool
aio_type_compat_open_mode(aio_type otyp, omode omod) {
  if (omod == omode::read_write) {
    return true;
  } else if (otyp == aio_type::read && omod == omode::read_only) {
    return true;
  } else if (otyp == aio_type::write && omod == omode::write_only) {
    return true;
  }
  return false;
}
}
}
