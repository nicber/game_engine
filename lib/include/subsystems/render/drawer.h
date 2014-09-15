#pragma once

namespace game_engine {
namespace logic {
namespace subsystems {
namespace render {
class drawer {
public:
  void draw(unsigned long long delta);
  bool try_combine_with(drawer &other);
};
}
}
}
}