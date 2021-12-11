
#ifndef GRAMODS_TRACK_TIMESAMPLEBUTTONSTRACKER
#define GRAMODS_TRACK_TIMESAMPLEBUTTONSTRACKER

#include <gmTrack/ButtonsTracker.hh>

#include <gmCore/OFactory.hh>

BEGIN_NAMESPACE_GMTRACK;

/**
   This buttons tracker reports button states as specified in the
   samples of the node.
 */
class TimeSampleButtonsTracker
  : public gmTrack::ButtonsTracker {

public:

  TimeSampleButtonsTracker();
  virtual ~TimeSampleButtonsTracker();

  /**
     Adds a time sample in seconds, from the start of the
     program. There must be at least two time samples as well as an
     equal amount of time samples and amount of button samples.

     \gmXmlTag{gmTrack,TimeSampleButtonsTracker,time}
  */
  void addTime(double t);

  /**
     Adds a button states sample as an integer where the least
     significant bit represents the main button.

     \gmXmlTag{gmTrack,TimeSampleButtonsTracker,buttons}
  */
  void addButtons(size_t b);

  /**
     Replaces the contents of p with button data.
  */
  bool getButtons(ButtonsSample &b) override;

  GM_OFI_DECLARE;

private:

  struct Impl;
  std::unique_ptr<Impl> _impl;

};

END_NAMESPACE_GMTRACK;

#endif
