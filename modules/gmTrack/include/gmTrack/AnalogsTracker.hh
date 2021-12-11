
#ifndef GRAMODS_TRACK_ANALOGSTRACKER
#define GRAMODS_TRACK_ANALOGSTRACKER

#include <gmTrack/config.hh>

#include <gmCore/Object.hh>

#include <chrono>
#include <vector>

BEGIN_NAMESPACE_GMTRACK;

/**
   The base of AnalogsTrackers
 */
class AnalogsTracker
  : public gmCore::Object {

public:

  typedef std::chrono::steady_clock clock;

  struct AnalogsSample {
    std::vector<float> analogs;
    clock::time_point time;
  };

  /**
     Returns the default key, in Configuration, for the
     Object.
  */
  virtual std::string getDefaultKey() override { return "analogsTracker"; }

  /**
     Replaces the contents of p with analogs data. Returns true if
     data could be read, false otherwise. Use sample time to check if
     data are fresh.
  */
  virtual bool getAnalogs(AnalogsSample &b) = 0;
};

END_NAMESPACE_GMTRACK;

#endif
