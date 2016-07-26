#pragma once

#include <cstddef>

namespace game_engine {
namespace thr_queue {
class allocated_stack;
namespace platform {
int SEH_filter_except_add_page(thr_queue::allocated_stack *stc, _EXCEPTION_POINTERS *ep);

struct allocated_stack {
  static bool commit_pages_prob();
  void plat_release();

  bool                                  pages_were_committed;
  size_t                                pages = 0;
};
}
}
}
