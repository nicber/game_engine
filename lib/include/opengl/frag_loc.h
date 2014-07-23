#pragma once

#include <GL/glew.h>

namespace game_engine {
namespace opengl {
class frag_loc {
public:
  frag_loc(GLuint loc);

private:
  GLuint f_data_loc;
};
}
}
