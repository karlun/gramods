
#ifndef GRAMODS_SOUND_CAPTURE
#define GRAMODS_SOUND_CAPTURE

#include <gmSound/config.hh>

#include <gmCore/OFactory.hh>

namespace gramods {
namespace gmSound {

/**
   The interface for a capture device that allow for extraction of
   sample data.
*/
class Capture : public gmCore::Object {

public:

  /**
     Returns the sample rate.
   */
  virtual size_t getSampleRate() = 0;

  /**
     Sets the number of channels to attempt to open for the capture
     device.
   */
  virtual size_t getChannelCount() = 0;

  virtual bool isOpen() = 0;

  virtual void startCapture() = 0;
  virtual void stopCapture() = 0;
  virtual size_t getAvailableSamplesCount() = 0;
  virtual std::vector<float> getAvailableSamples() = 0;

  std::string getDefaultKey() override { return "capture"; }

  /**
     De-interlaces the input sample data into an array of N vectors
     each with an Nth of the original samples.
  */
  template<size_t N>
  static std::array<std::vector<float>, N>
  deinterlaceSamples(std::vector<float> data);

  /**
     De-interlaces the input sample data into an array of N vectors
     each with an Nth of the original samples.
  */
  static std::vector<std::vector<float>>
  deinterlaceSamples(std::vector<float> data, size_t N);
};
}}

#endif
