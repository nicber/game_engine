#pragma once
#include <WinSock2.h>
#include <Windows.h>

namespace game_engine {
namespace aio {
struct aio_operation_platform {

};

struct aio_result_platform {
  OVERLAPPED overlapped;
};
}
}