
#include <gmSound/OpenALContext.hh>

#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alext.h>

#include <gmCore/Console.hh>

#include <optional>

namespace gramods {
namespace gmSound {

GM_OFI_DEFINE(OpenALContext);
GM_OFI_PARAM2(OpenALContext, deviceName, std::string, setDeviceName);

struct OpenALContext::Impl {
  ~Impl();

  void open();

  std::optional<std::string> device_name;

  ALCdevice *device = nullptr;
  ALCcontext *context = nullptr;
};

OpenALContext::OpenALContext() : _impl(std::make_unique<Impl>()) {}
OpenALContext::~OpenALContext() {}

void OpenALContext::initialize() {
  _impl->open();
}

void OpenALContext::Impl::open() {

  if (!device_name)
    device_name = alcGetString(NULL, ALC_DEFAULT_DEVICE_SPECIFIER);

  device = alcOpenDevice(device_name->c_str());
  if (!device) {
    GM_ERR("OpenALContext",
           "Could not open audio device '" << *device_name << "'!");
    return;
  }

  context = alcCreateContext(device, NULL);
  if (context == NULL) {
    alcCloseDevice(device);
    GM_ERR("OpenALContext", "Could not create a context!");
    return;
  }

  if (alcMakeContextCurrent(context) == ALC_FALSE) {
    alcDestroyContext(context);
    alcCloseDevice(device);
    GM_ERR("OpenALContext", "Could not make context current!");
    return;
  }

  const ALCchar *name = NULL;
  if (alcIsExtensionPresent(device, "ALC_ENUMERATE_ALL_EXT"))
    name = alcGetString(device, ALC_ALL_DEVICES_SPECIFIER);
  if (!name || alcGetError(device) != AL_NO_ERROR)
    name = alcGetString(device, ALC_DEVICE_SPECIFIER);
  GM_DBG1("OpenALContext", "Opened '" << name << "'");
}

OpenALContext::Impl::~Impl() {
  if (context == NULL) return;
  if (alcGetCurrentContext() == context) alcMakeContextCurrent(NULL);
  alcDestroyContext(context);
  alcCloseDevice(device);
}

void OpenALContext::setDeviceName(std::string name) {
  _impl->device_name = name;
}

bool OpenALContext::isOpen() { return _impl->context != nullptr; }

std::vector<std::string> OpenALContext::getDeviceNames() {
  
  std::vector<std::string> list;

  auto data = alcGetString(NULL, ALC_DEVICE_SPECIFIER);
  while (true) {
    list.push_back(std::string(data));
    data += list.back().size() + 1;
    if (data[0] == '\0') break;
  }
  return list;
}

void OpenALContext::makeCurrent() {
  if (_impl->context == nullptr) return;
  alcMakeContextCurrent(_impl->context);
}

}}
