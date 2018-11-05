
#ifndef GRAMODS_TRACK_BUTTONSTRACKER
#define GRAMODS_TRACK_BUTTONSTRACKER

#include <gmTrack/config.hh>

#include <gmCore/Object.hh>
#include <chrono>

BEGIN_NAMESPACE_GMTRACK;

/**
   The base of ButtonsTrackers
 */
class ButtonsTracker
  : public gmCore::Object {

public:

  typedef std::chrono::steady_clock clock;

  struct ButtonsSample {
    bool main_button;
    unsigned int buttons;
    clock::time_point time;
  };

  /**
     Returns the default key, in Configuration, for the
     Object.
  */
  virtual std::string getDefaultKey() { return "buttonsTracker"; }

  /**
     Replaces the contents of p with button data.
  */
  virtual bool getButtons(ButtonsSample &b) = 0;
};

END_NAMESPACE_GMTRACK;

#endif
