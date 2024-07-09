
#ifndef GRAMODS_SOUND_MULTILATERATION
#define GRAMODS_SOUND_MULTILATERATION

// Required before gmCore/OFactory.hh for some compilers
#include <gmCore/io_eigen.hh>

#include <gmSound/SoundDetector.hh>

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

     \gmXmlTag{gmSound,Multilateration,soundDetector}
  */
  void setSoundDetector(std::shared_ptr<SoundDetector>);

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

  /**
     Propagates the specified visitor.

     @see Object::Visitor
  */
  void traverse(Visitor *visitor) override;

  GM_OFI_DECLARE;

private:
  struct Impl;
  std::unique_ptr<Impl> _impl;
};
}
}

#endif
