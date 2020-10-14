
#include <gmCore/TimeTools.hh>

BEGIN_NAMESPACE_GMCORE;

TimeTools::clock::time_point TimeTools::secondsToTimePoint(double s) {
  typedef std::chrono::duration<double, std::ratio<1>> d_seconds;

  d_seconds since_epoch_full(s);
  auto since_epoch = std::chrono::duration_cast<clock::duration>(since_epoch_full);

  return clock::time_point(since_epoch);
}

double TimeTools::timePointToSeconds(clock::time_point t) {
  return durationToSeconds(t.time_since_epoch());
}

double TimeTools::durationToSeconds(clock::duration t) {
  typedef std::chrono::duration<double, std::ratio<1>> d_seconds;
  auto since_epoch = std::chrono::duration_cast<d_seconds>(t);
  return since_epoch.count();
}

TimeTools::clock::duration TimeTools::secondsToDuration(double s) {
  typedef std::chrono::duration<double, std::ratio<1>> d_seconds;
  return std::chrono::duration_cast<clock::duration>(d_seconds(s));
}

END_NAMESPACE_GMCORE;
