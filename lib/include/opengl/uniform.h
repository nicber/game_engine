#pragma once

#include <GL/glew.h>

namespace game_engine {
namespace opengl {
class uniform {
public:
  uniform(GLint loc);

private:
  GLint uniform_loc;
};
}
}
