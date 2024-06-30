
#ifndef GRAMODS_SOUND_SOUNDBURSTDETECTOR
#define GRAMODS_SOUND_SOUNDBURSTDETECTOR

#include <gmSound/SoundDetector.hh>

namespace gramods {
namespace gmSound {

/**
   Sound detector search for samples over a specified threshold.
*/
class SoundBurstDetector : public SoundDetector {

public:

  SoundBurstDetector();
  ~SoundBurstDetector();

  /**
     Sets the threshold used for detecting a sound event in the range
     0-1. Default is 70%.

     \gmXmlTag{gmSound,SoundBurstDetector,threshold}
  */
  void setThreshold(float r);

  /**
     Sets the cooldown time for a detection, i.e. the time between two
     samples over the threshold that is considered another
     sound. Default is 0.1 seconds.

     \gmXmlTag{gmSound,SoundBurstDetector,cooldown}
  */
  void setCooldown(float t);

  /**
     Reads off new samples from the capture device, if such are
     available, then detect sound based over the specified threshold,
     estimating how long ago the sound appeared in each channel.

     \see SoundDetector::detectSound
  */
  std::vector<std::vector<float>> detectSound() override;

  GM_OFI_DECLARE;

private:
  struct Impl;
  std::unique_ptr<Impl> _impl;
};

}}

#endif
