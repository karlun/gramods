
#ifndef GRAMODS_TRACK_BUTTONSTRACKER
#define GRAMODS_TRACK_BUTTONSTRACKER

#include <gmTrack/config.hh>

#include <gmCore/Object.hh>

#include <chrono>
#include <map>

BEGIN_NAMESPACE_GMTRACK;

/**
   The base of ButtonsTrackers, reporting button states.
*/
class ButtonsTracker
  : public gmCore::Object {

public:

  typedef std::chrono::steady_clock clock;

  /**
     A sample containing the state of any number of buttons.
  */
  struct ButtonsSample {

    /**
       The state of the buttons communicated.
    */
    std::map<size_t, bool> buttons;

    /**
       The time of the sample.
    */
    clock::time_point time;
  };

  /**
     Returns the default key, in Configuration, for the
     Object.
  */
  virtual std::string getDefaultKey() override { return "buttonsTracker"; }

  /**
     Replaces the contents of p with button data. Returns true if data
     could be read, false otherwise. Use sample time to check if data
     are fresh.
  */
  virtual bool getButtons(ButtonsSample &b) = 0;
};

END_NAMESPACE_GMTRACK;

#endif
