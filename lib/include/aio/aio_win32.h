#pragma once

namespace game_engine {
namespace aio {
namespace platform {
class perform_helper_impl {
  bool apc_pending_exec = false;
protected:
  void plat_about_to_block();
  void plat_cant_block_anymore();
};
}
}
}