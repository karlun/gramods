
#ifndef GRAMODS_SOUND_MULTILATERATION
#define GRAMODS_SOUND_MULTILATERATION

#include <gmSound/Capture.hh>

#include <gmCore/Updateable.hh>
#include <gmTrack/SinglePoseTracker.hh>

namespace gramods {
namespace gmSound {

/**
   Estimates the position of sound.
*/
class Multilateration : public gmTrack::SinglePoseTracker,
                         public gmCore::Updateable {

public:
  typedef gmCore::Updateable::clock clock;

  Multilateration();
  virtual ~Multilateration();

  /**
     Adds a microphone position for the multilateration. The number of
     microphone positions should match the number of channels in the
     capture.

     \gmXmlTag{gmSound,Multilateration,point}
  */
  void addPoint(Eigen::Vector3f pt);

  /**
     Sets the threshold used for detecting a sound event in the range
     0-1. Default is 70%.

     \gmXmlTag{gmSound,Multilateration,threshold}
  */
  void setThreshold(float r);

  /**
     Sets the time window (in seconds) over which to search for
     matching events. Default is 0.03 seconds, which corresponds to
     approximately 10 meters with the default speed of sound.

     \gmXmlTag{gmSound,Multilateration,window}
  */
  void setWindow(float t);

  /**
     Sets the speed of sound (m/s) to use when estimating the
     distances from the sound. Default is 343 m/s, which is the speed
     of sound in air at 20 °C.

     \gmXmlTag{gmSound,Multilateration,speedOfSound}
  */
  void setSpeedOfSound(float v);

  /**
     Sets the capture object to read audio sample data from.  The number of
     microphone positions should match the number of channels in the
     capture.

     \gmXmlTag{gmSound,Multilateration,capture}
  */
  void setCapture(std::shared_ptr<Capture>);

  /**
     Replaces the contents of p with pose data. Use sample time to check if
     data are fresh.
  */
  bool getPose(PoseSample &p) override;

  /**
     Called by updateAll to perform the multilateration and set the
     most current position, if one is detected.
  */
  void update(clock::time_point t, size_t frame) override;

  void initialize() override;

  GM_OFI_DECLARE;

private:
  struct Impl;
  std::unique_ptr<Impl> _impl;
};
}
}

#endif
