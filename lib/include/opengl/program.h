#pragma once

#include "opengl/shader.h"

namespace game_engine {
namespace opengl {
class program_construction_error : public std::exception {
public:
  using std::logic_error::login_error;
  program_construction_error(std::string error,
                             std::initializer_list<shader> shaders);
};

class program {
public:
  program(std::initializer_list<shader> shaders);
  
private:
  
};
}
}