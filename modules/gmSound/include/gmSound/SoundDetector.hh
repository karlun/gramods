
#ifndef GRAMODS_SOUND_SOUNDDETECTOR
#define GRAMODS_SOUND_SOUNDDETECTOR

#include <gmSound/config.hh>

#include <gmSound/Capture.hh>

#include <deque>

namespace gramods {
namespace gmSound {

/**
   The interface for an algorithm detecting sound in an audio stream.
*/
class SoundDetector : public gmCore::Object {

public:

  SoundDetector();
  ~SoundDetector();

  /**
     Returns the sample rate.
   */
  size_t getSampleRate();

  /**
     Sets the number of channels to attempt to open for the capture
     device.
   */
  size_t getChannelCount();

  /**
     Sets the capture object to read audio sample data from. The number of
     microphone positions should match the number of channels in the
     capture.

     \gmXmlTag{gmSound,SoundDetector,capture}
  */
  void setCapture(std::shared_ptr<Capture> c);

  /**
     Sets the time window (in seconds) over which to search for
     matching events. Default is 0.029 seconds, which corresponds to
     approximately 10 meters with the default speed of sound.

     \gmXmlTag{gmSound,SoundDetector,window}
  */
  void setWindow(float t);

  /**
     Calls getData and applies the implementation specific algorithm
     to these data to detect sound in the stream, estimating how long
     ago the sound appeared in each channel.

     \returns The time offset, in seconds, of the t0 of all detected
     sounds, per channel. Zero equals now and positive values
     represent back in time. The outer vector should always have size
     equal to the number of channels, even if no sound was detected.
  */
  virtual std::vector<std::vector<float>> detectSound() = 0;

  std::string getDefaultKey() override { return "soundDetector"; }

  GM_OFI_DECLARE;

protected:
  /**
     Fetches the current data that span the specified window. The
     referenced object is valid at least until the getData method is
     called again.
  */
  const std::vector<std::deque<float>> &getData();

private:
  struct Impl;
  std::unique_ptr<Impl> _impl;
};

}}

#endif
