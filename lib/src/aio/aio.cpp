#include <aio/aio.h>

namespace game_engine {
namespace aio {
aio_result_t::aio_result_t()
 :finished(false)
 ,succeeded(false)
 ,read_bytes(0)
{}
}
}
