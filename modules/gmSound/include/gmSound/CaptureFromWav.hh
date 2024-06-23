
#ifndef GRAMODS_SOUND_CAPTUREFROMWAV
#define GRAMODS_SOUND_CAPTUREFROMWAV

#include <gmSound/Capture.hh>

#include <memory>

namespace gramods {
namespace gmSound {

/**
   The instantiation of this class will attempt to open a capture
   device and allow for extraction of sample data from this.
*/
class CaptureFromWav : public gmSound::Capture {

public:
  CaptureFromWav();
  virtual ~CaptureFromWav();

  void setFile(std::filesystem::path file);
  void setLoop(bool on);
  void setExit(bool on);

  size_t getSampleRate() override;
  size_t getChannelCount() override;

  bool isOpen() override;

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
