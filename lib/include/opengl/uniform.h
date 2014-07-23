#pragma once

#include <GL/glew.h>

namespace game_engine {
namespace opengl {
class uniform {
public:
  uniform(GLuint loc);

private:
  GLuint uniform_loc;
};
}
}
