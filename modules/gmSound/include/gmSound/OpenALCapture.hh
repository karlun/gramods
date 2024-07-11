
#ifndef GRAMODS_SOUND_OPENALCAPTURE
#define GRAMODS_SOUND_OPENALCAPTURE

// Required before gmCore/OFactory.hh for some compilers
#include <gmCore/io_size.hh>

#include <gmSound/Capture.hh>

#include <memory>

namespace gramods {
namespace gmSound {

/**
   The instantiation of this class will attempt to open a capture
   device and allow for extraction of sample data from this.
*/
class OpenALCapture : public gmSound::Capture {

public:
  OpenALCapture();
  virtual ~OpenALCapture();

  /**
     Sets the name of the capture device to open and capture from.

     \gmXmlTag{gmSound,OpenALCapture,deviceName}
  */
  void setDeviceName(std::string name);

  /**
     Sets the sample rate to attempt to open the capture device
     with. Default is 44100 samples per second.

     \gmXmlTag{gmSound,OpenALCapture,sampleRate}
   */
  void setSampleRate(size_t f);
  size_t getSampleRate() override;

  /**
     Sets the number of channels to attempt to open for the capture
     device.

     \gmXmlTag{gmSound,OpenALCapture,channelCount}
   */
  void setChannelCount(size_t n);
  size_t getChannelCount() override;

  bool isOpen() override;

  static std::vector<std::string> getCaptureDeviceNames();

  void startCapture() override;
  void stopCapture() override;
  std::vector<float> getAvailableSamples() override;
  size_t getAvailableSamplesCount() override;

  void initialize() override;

  GM_OFI_DECLARE;

private:
  struct Impl;
  std::unique_ptr<Impl> _impl;
};
}
}

#endif
