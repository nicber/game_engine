#pragma once

#include <GL/glew.h>

namespace game_engine {
namespace opengl {
class frag_loc {
public:
  frag_loc(GLint loc);

private:
  GLint f_data_loc;
};
}
}
