#include "subsystems/time.h"
#include "time_access.h"

namespace game_engine {
namespace logic {
namespace subsystems {
void
time_subsystem::after_start_tick( )
{
  auto curr_time = read_absolute_time( );
  if ( !start_time_already_set ) {
    start_time             = curr_time;
    start_time_already_set = true;
  }
  previous_time = current_time;
  current_time  = curr_time - start_time;
}

time
time_subsystem::absolute( ) const
{
  return current_time;
}

bool
time_subsystem::accepts( component& )
{
  return false;
}

void
time_subsystem::update_all( )
{
  // Do nothing.
}

time
time_subsystem::us_since_last_tick( ) const
{
  return current_time - previous_time;
}
}
}
}