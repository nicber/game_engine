#pragma once

#include <uv.h>

namespace game_engine {
namespace thr_queue {
namespace event {
using uv_async_cb = void(uv_async_t *, int);
void uv_thr_init(uv_async_t *async, uv_async_cb *f_ptr);
}
}
}
