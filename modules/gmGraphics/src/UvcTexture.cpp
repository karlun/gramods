
#include <gmGraphics/UvcTexture.hh>

#ifdef gramods_ENABLE_libuvc

#include <libuvc/libuvc.h>
#include <stdio.h>
#include <thread>
#include <sstream>

#include <glbinding/gl/enum.h>
#include <glm/glm.hpp>

BEGIN_NAMESPACE_GMGRAPHICS;

GM_OFI_DEFINE(UvcTexture);
GM_OFI_PARAM(UvcTexture, vendor, int, UvcTexture::setVendor);
GM_OFI_PARAM(UvcTexture, product, int, UvcTexture::setProduct);
GM_OFI_PARAM(UvcTexture, serial, std::string, UvcTexture::setSerial);
GM_OFI_PARAM(UvcTexture, resolution, gmTypes::size2, UvcTexture::setResolution);
GM_OFI_PARAM(UvcTexture, framerate, int, UvcTexture::setFramerate);
GM_OFI_PARAM(UvcTexture, format, std::string, UvcTexture::setFormat);


struct UvcTexture::_This {

  _This();
  ~_This();

  static uvc_frame_format formatFromString(std::string s);

  bool initialize_context();
  bool locate_device(int vendor, int product, std::string serial);
  bool open_device();
  bool negotiate_format();
  bool start_streaming();

  static void uvc_frame_cb(uvc_frame_t *frame, void *ptr);

  void startAll(int vendor, int product, std::string serial);
  void closeAll();

  void update();
  globjects::Texture * getTexture() { return texture.get(); }

  uvc_context_t *context = nullptr;
  uvc_device_t *device = nullptr;
  uvc_device_handle_t *device_handle = nullptr;
  uvc_stream_ctrl_t stream_control;

  gmTypes::size2 resolution;
  int framerate;
  uvc_frame_format format;

  uvc_frame_t *cache_bgr = nullptr;

  std::unique_ptr<globjects::Texture> texture;
  bool started = false;
};

UvcTexture::UvcTexture()
  : _this(new _This()),
    vendor(0),
    product(0),
    serial("") {}

void UvcTexture::initialize() {
  LiveTexture::initialize();
}

UvcTexture::_This::_This()
  : context(nullptr),
    device(nullptr),
    device_handle(nullptr) {}

UvcTexture::_This::~_This() {
  closeAll();
}

void UvcTexture::update() {
  if (!_this->started) {
    _this->startAll(vendor, product, serial);
    _this->started = true;
  }

  _this->update();
}

globjects::Texture * UvcTexture::getTexture() {
  return _this->getTexture();
}

void UvcTexture::_This::startAll(int vendor, int product, std::string serial) {
  if (!initialize_context()) return;
  if (!locate_device(vendor, product, serial)) return;
  if (!open_device()) return;
  if (!negotiate_format()) return;
  if (!start_streaming()) return;
}

void UvcTexture::_This::update() {

  if (!cache_bgr)
    return;

  if (!texture)
    try {
      texture = std::make_unique<globjects::Texture>();
    } catch(std::exception e) {
      GM_ERR("UvcTexture", "Could not create GL texture: " << e.what());
      closeAll();
      return;
    }

  // TODO: Fill texture with data
  // uvc_frame_t cache_bgr uvc_frame_t
  cache_bgr->data;
  cache_bgr->width;
  cache_bgr->height;
  cache_bgr->data_bytes;

  if (cache_bgr->data_bytes > cache_bgr->width * cache_bgr->height * 3) {
    GM_WRN("UvcTexture", "Too much data in frame");
  }

  if (cache_bgr->data_bytes < cache_bgr->width * cache_bgr->height * 3) {
    GM_ERR("UvcTexture", "Too little data in frame");
    return;
  }

  texture->image2D(0, gl::GL_RGB, glm::ivec2(cache_bgr->width, cache_bgr->height),
                   0, gl::GL_BGR, gl::GL_UNSIGNED_BYTE, cache_bgr->data);
}

bool UvcTexture::_This::initialize_context() {
  uvc_error_t res = uvc_init(&context, NULL);
  if (res < 0) {
    closeAll();

    GM_ERR("UvcTexture", "Cannot initialize UVC context: " << uvc_strerror(res));
    return false;
  }
  GM_INF("UvcTexture", "UVC context successfully initialized");
  return true;
}

bool UvcTexture::_This::locate_device(int vendor, int product, std::string serial) {
  const char *serial_cstr = serial.size() == 0 ? NULL : &serial[0];
  uvc_error_t res = uvc_find_device(context, &device, vendor, product, serial_cstr);
  if (res < 0) {
    closeAll();

    GM_ERR("UvcTexture", "Cannot find UVC device (vendor=" << vendor << ", product=" << product << ", serial=" << serial << "): " << uvc_strerror(res));
    return false;
  }
  GM_INF("UvcTexture", "Found UVC device (vendor=" << vendor << ", product=" << product << ", serial=" << serial << ")");
  return true;
}

bool UvcTexture::_This::open_device() {
  /* Try to open the device: requires exclusive access */
  uvc_error_t res = uvc_open(device, &device_handle);
  if (res < 0) {
    closeAll();

    GM_ERR("UvcTexture", "Unable to open UVC device: " << uvc_strerror(res));
    return false;
  }
  GM_INF("UvcTexture", "Opened UVC device");
  return true;
}

bool UvcTexture::_This::negotiate_format() {

  /* Print out a message containing all the information that libuvc
   * knows about the device */
  uvc_print_diag(device_handle, stderr);

  uvc_error_t res = uvc_get_stream_ctrl_format_size
    (device_handle, &stream_control,
     format, resolution[0], resolution[1], framerate);

  /* Print out the result */
  uvc_print_stream_ctrl(&stream_control, stderr);
  if (res < 0) {
    closeAll();

    GM_ERR("UvcTexture", "Unable to nagotiate UVC format: " << uvc_strerror(res));
    return false;
  }
  GM_INF("UvcTexture", "Successful UVC format negotiation");
  return true;
}

bool UvcTexture::_This::start_streaming() {
  uvc_error_t res = uvc_start_streaming(device_handle, &stream_control, &UvcTexture::_This::uvc_frame_cb, this, 0);
  if (res < 0) {
    closeAll();

    uvc_perror(res, "start_streaming"); /* unable to start stream */
    std::stringstream msg;
    msg << "Unable to start stream: "
        << uvc_strerror(res);
    throw std::runtime_error(msg.str());
  }
  GM_INF("UvcTexture", "Started UVC streaming");
  return true;
}

/* This callback function runs once per frame. Use it to perform any
 * quick processing you need, or have it put the frame into your application's
 * input queue. If this function takes too long, you'll start losing frames. */
void UvcTexture::_This::uvc_frame_cb(uvc_frame_t *frame, void *ptr) {

  UvcTexture::_This *_this = static_cast<UvcTexture::_This*>(ptr);

  uvc_frame_t *bgr;
  uvc_error_t ret;

  /* We'll convert the image from YUV/JPEG to BGR, so allocate space */
  bgr = uvc_allocate_frame(frame->width * frame->height * 3);
  if (!bgr) {
    printf("unable to allocate bgr frame!");
    return;
  }

  /* Do the BGR conversion */
  ret = uvc_any2bgr(frame, bgr);
  if (ret) {
    uvc_perror(ret, "uvc_any2bgr");
    uvc_free_frame(bgr);
    return;
  }

  // TODO: Queue up frame data so that GL thread may call image2D for them
  if (_this->cache_bgr) uvc_free_frame(_this->cache_bgr);
  _this->cache_bgr = bgr;
}

void UvcTexture::_This::closeAll() {
  if (cache_bgr)
    uvc_free_frame(cache_bgr);
  if (device_handle != nullptr) {
    uvc_stop_streaming(device_handle);
    uvc_close(device_handle);
  }
  if (device != nullptr)
    uvc_unref_device(device);
  if (context != nullptr)
    uvc_exit(context);

  device_handle = nullptr;
  device = nullptr;
  context = nullptr;

  cache_bgr = nullptr;
  texture = nullptr;
}

void UvcTexture::setResolution(gmTypes::size2 res) {
  _this->resolution = res;
}

void UvcTexture::setFramerate(int fps) {
  _this->framerate = fps;
}

void UvcTexture::setFormat(std::string fmt) {
  _this->format = UvcTexture::_This::formatFromString(fmt);
}

uvc_frame_format UvcTexture::_This::formatFromString(std::string s) {

#define FORMAT(A,B)                                   \
  if (std::equal(s.begin(), s.end(),                  \
                 #B, #B + strlen(#B),                 \
                 [](char a, char b) {                 \
                   return tolower(a) == tolower(b);   \
                 })) return A; else //

  FORMAT(UVC_FRAME_FORMAT_ANY, ANY);
  FORMAT(UVC_FRAME_FORMAT_UNCOMPRESSED, UNCOMPRESSED);
  FORMAT(UVC_FRAME_FORMAT_COMPRESSED, COMPRESSED);
  FORMAT(UVC_FRAME_FORMAT_YUYV, YUYV);
  FORMAT(UVC_FRAME_FORMAT_UYVY, UYVY);
  FORMAT(UVC_FRAME_FORMAT_RGB, RGB);
  FORMAT(UVC_FRAME_FORMAT_BGR, BGR);
  FORMAT(UVC_FRAME_FORMAT_MJPEG, MJPEG);
  FORMAT(UVC_FRAME_FORMAT_GRAY8, GRAY8);
  FORMAT(UVC_FRAME_FORMAT_GRAY16, GRAY16);
  FORMAT(UVC_FRAME_FORMAT_BY8, BY8);
  FORMAT(UVC_FRAME_FORMAT_BA81, BA81);
  FORMAT(UVC_FRAME_FORMAT_SGRBG8, SGRBG8);
  FORMAT(UVC_FRAME_FORMAT_SGBRG8, SGBRG8);
  FORMAT(UVC_FRAME_FORMAT_SRGGB8, SRGGB8);
  FORMAT(UVC_FRAME_FORMAT_SBGGR8, SBGGR8);
  {
    GM_WRN("UvcTexture", "Unrecognized frame format " << s << "; using any");
    return UVC_FRAME_FORMAT_ANY;
  }

#undef FORMAT
}

END_NAMESPACE_GMGRAPHICS;

#endif