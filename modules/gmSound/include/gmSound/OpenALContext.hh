
#ifndef GRAMODS_SOUND_OPENALCONTEXT
#define GRAMODS_SOUND_OPENALCONTEXT

#include <gmSound/config.hh>

#include <gmCore/OFactory.hh>

#include <memory>

namespace gramods {
namespace gmSound {

/**
 * The instantiation of this class will attempt to open a (specified)
 * playback device and create a context for it.
 */
class OpenALContext : public gmCore::Object {

public:
  OpenALContext();
  virtual ~OpenALContext();

  /**
     Sets the name of the playback device to open a context for.

     \gmXmlTag{gmSound,OpenALContext,deviceName}
  */
  void setDeviceName(std::string name);

  bool isOpen();

  static std::vector<std::string> getDeviceNames();

  /**
     Makes this context the current for subsequent calls.
  */
  void makeCurrent();

  std::string getDefaultKey() override { return "openALContext"; }
  void initialize() override;

  GM_OFI_DECLARE;

private:
  struct Impl;
  std::unique_ptr<Impl> _impl;
};

}
}

#endif
