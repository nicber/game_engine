#include "platform/time/win32.h"
#include "time_access.h"

namespace game_engine {
namespace logic {
time
read_absolute_time( )
{
  LARGE_INTEGER tim;
  QueryPerformanceCounter( &tim );
  return { time::type::platform, tim.QuadPart };
}

namespace platform {
static LARGE_INTEGER frequency = []( ) {
  LARGE_INTEGER res;
  QueryPerformanceFrequency( &res );
  return res;
}( );

time_type
from_us( long long us ) no_except
{
  return ( us / 1000 ) * ( frequency.QuadPart / 1000 );
}

long long
to_us( time_type tt ) no_except
{
  return tt * ( 1000 * 1000 / frequency.QuadPart );
}
}
}
}