#pragma once

namespace game_engine {
namespace logic {
namespace subsystems {
namespace render {
class drawer {
public:
  virtual void draw(unsigned long long delta) = 0;
  virtual bool try_combine_with(drawer &other) = 0;
};
}
}
}
}
