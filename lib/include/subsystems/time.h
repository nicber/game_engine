#pragma once

#include "subsystem.h"

namespace game_engine {
namespace logic {
namespace subsystems {
/** \brief Class that represents a subsystem that can inform the time.
 * It should be added to a game so other subsystems can know the time delta
 * between different ticks.
 */
class time_subsystem : public subsystem
{
  /** \brief Updated after the call to after_start_tick().
   * It is returned by absolute().
   */
  time current_time;

  /** \brief Stores the previous current_time. */
  time previous_time;

  /** \brief Stores time returned by read_absolute_time the first
   * time.
   */
  time start_time;

  /** \Used as a flags for knowing if the value of start_time is
   * valid or needs to be set.
   */
  bool start_time_already_set = false;

  /** \brief Updates the internal clock. */
  void after_start_tick( ) override;

  /** \brief Rejects all components. */
  bool accepts( component& comp ) final override;

  /** \brief Does nothing. */
  void update_all( ) final override;

public:
  /** \brief Returns the time since the first tick. */
  time absolute( ) const;

  /** \brief Returns the time between this tick and the previous one.
   * It is supposed to be used for getting the time delta between frames.
   */
  time us_since_last_tick( ) const;
};
}
}
}