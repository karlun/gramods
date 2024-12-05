
#ifndef GRAMODS_CORE_TIMETOOLS
#define GRAMODS_CORE_TIMETOOLS

#include <gmCore/config.hh>

#include <chrono>

BEGIN_NAMESPACE_GMCORE;

/**
   Static functions for converting time. For example, to find the time
   in seconds used in a certain call:

   ```
   typedef gmCore::TimeTools::clock clock;
   clock::time_point t0 = clock::now();
   expensive_call_to_measure();
   clock::time_point t1 = clock::now();
   clock::duration time_span = t1 - t0;
   std::cerr << "Time used: "
             << gmCore::TimeTools::durationToSeconds(time_span)
             << std::endl;
   ```
*/
struct TimeTools {

  typedef std::chrono::steady_clock clock;

  /**
   * Converts the specified time (in seconds) from epoch to the
   * steady_clock time_point type.
   */
  static clock::time_point secondsToTimePoint(double s);

  /**
   * Converts the specified time from the steady_clock time_point into
   * seconds from epoch.
   */
  static double timePointToSeconds(clock::time_point t);

  /**
   * Converts the specified duration from the steady_clock duration
   * format into seconds.
   */
  static double durationToSeconds(clock::duration t);

  /**
   * Converts the specified time (in seconds) into the steady_clock
   * duration type.
   */
  static clock::duration secondsToDuration(double s);
};

END_NAMESPACE_GMCORE;

#endif
