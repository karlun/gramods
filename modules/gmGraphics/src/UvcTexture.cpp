
#include <gmGraphics/UvcTexture.hh>

#ifdef gramods_ENABLE_libuvc

#include <gmCore/RunOnce.hh>
#include <gmCore/Console.hh>

#include <libuvc/libuvc.h>
#include <stdio.h>
#include <string.h>
#include <thread>
#include <vector>
#include <sstream>
#include <mutex>
#include <cctype>

BEGIN_NAMESPACE_GMGRAPHICS;

GM_OFI_DEFINE(UvcTexture);
GM_OFI_PARAM2(UvcTexture, vendor, int, setVendor);
GM_OFI_PARAM2(UvcTexture, product, int, setProduct);
GM_OFI_PARAM2(UvcTexture, serial, std::string, setSerial);
GM_OFI_PARAM2(UvcTexture, resolution, gmCore::size2, setResolution);
GM_OFI_PARAM2(UvcTexture, framerate, int, setFramerate);
GM_OFI_PARAM2(UvcTexture, format, std::string, setFormat);
GM_OFI_PARAM2(UvcTexture, convertToRgb, bool, setConvertToRbg);


struct UvcTexture::Impl {

  Impl();
  ~Impl();

  static uvc_frame_format formatFromString(std::string s);
  static std::string formatToString(uvc_frame_format f);

  bool initialize_context();
  bool locate_device(int vendor, int product, std::string serial);
  bool open_device();
  bool negotiate_format();
  bool start_streaming();

  static void uvc_frame_cb(uvc_frame_t *frame, void *ptr);

  void startAll(int vendor, int product, std::string serial);
  void closeAll();

  void update();
  GLuint getGLTextureID() { return texture_id; }

  uvc_context_t *context = nullptr;
  uvc_device_t *device = nullptr;
  uvc_device_handle_t *device_handle = nullptr;
  uvc_stream_ctrl_t stream_control;

  gmCore::size2 resolution = { 640, 480 };
  int framerate = 30;
  uvc_frame_format format = formatFromString("any");
  bool convert_to_rgb = true;

  uvc_frame_t *frame_cache = nullptr;
  std::mutex data_lock;

  GLuint texture_id = 0;
  bool started = false;
};

UvcTexture::UvcTexture()
  : _impl(new Impl()),
    vendor(0),
    product(0),
    serial("") {}

void UvcTexture::initialize() {
  Texture::initialize();
}

UvcTexture::Impl::Impl()
  : context(nullptr),
    device(nullptr),
    device_handle(nullptr) {}

UvcTexture::Impl::~Impl() {
  closeAll();
}

void UvcTexture::update() {
  if (!_impl->started) {
    _impl->startAll(vendor, product, serial);
    _impl->started = true;
  }

  _impl->update();
}

GLuint UvcTexture::getGLTextureID() {
  return _impl->getGLTextureID();
}

void UvcTexture::Impl::startAll(int vendor, int product, std::string serial) {
  if (!initialize_context()) return;
  if (!locate_device(vendor, product, serial)) return;
  if (!open_device()) return;
  if (!negotiate_format()) return;
  if (!start_streaming()) return;
}

void UvcTexture::Impl::update() {

  if (!texture_id) {
    std::vector<GLubyte> data;
    for (size_t idxY = 0; idxY < resolution[1]; ++idxY)
      for (size_t idxX = 0; idxX < resolution[0]; ++idxX)
        for (size_t idxC = 0; idxC < 3; ++idxC)
          data.push_back(((idxX/10 + idxY/10) % 2 ? 150 : 50) +
                         ((idxX/100 + idxY/100) % 2 ? 20 : -20));

    glGenTextures(1, &texture_id);
    glBindTexture(GL_TEXTURE_2D, texture_id);
    glTexImage2D(GL_TEXTURE_2D,
                 0, GL_RGBA8, resolution[0], resolution[1],
                 0, GL_RGB, GL_UNSIGNED_BYTE, &data.front());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);
    GM_DBG1("UvcTexture", "Created texture with default data");
  }

  std::lock_guard<std::mutex> guard(data_lock);

  if (!frame_cache)
    return;

  if (convert_to_rgb) {

    GM_DBG2("UvcTexture", "Converting frame cache to RGB.");

    uvc_frame_t *rgb;

    rgb = uvc_allocate_frame(frame_cache->width * frame_cache->height * 3);
    if (!rgb) {
      GM_ERR("UvcTexture", "Unable to allocate rgb frame");
      return;
    }

    uvc_error_t ret =
      frame_cache->frame_format == UVC_FRAME_FORMAT_MJPEG ?
      uvc_mjpeg2rgb(frame_cache, rgb) :
      uvc_any2rgb(frame_cache, rgb);
    if (ret) {
      GM_ERR("UvcTexture", "Cannot convert incoming data "
             << "(" << formatToString(frame_cache->frame_format) << "): "
             << uvc_strerror(ret));
      uvc_free_frame(rgb);
      return;
    }

    uvc_free_frame(frame_cache);
    frame_cache = nullptr;

    if (rgb->data_bytes > rgb->width * rgb->height * 3) {
      GM_WRN("UvcTexture", "Too much data in frame");
    }

    if (rgb->data_bytes < rgb->width * rgb->height * 3) {
      GM_ERR("UvcTexture", "Too little data in frame");
      return;
    }

    glBindTexture(GL_TEXTURE_2D, texture_id);
    glTexImage2D(GL_TEXTURE_2D,
                 0, GL_RGB, rgb->width, rgb->height,
                 0, GL_RGB, GL_UNSIGNED_BYTE, rgb->data);
    glBindTexture(GL_TEXTURE_2D, 0);

    uvc_free_frame(rgb);

  } else if (frame_cache->frame_format == UVC_FRAME_FORMAT_YUYV ||
             frame_cache->frame_format == UVC_FRAME_FORMAT_UYVY) {
    // Two different formats with identical macropixel size

    if (frame_cache->data_bytes > frame_cache->width * frame_cache->height * 2) {
      GM_WRN("UvcTexture", "Too much data in frame");
    }

    if (frame_cache->data_bytes < frame_cache->width * frame_cache->height * 2) {
      GM_ERR("UvcTexture", "Too little data in frame");
      return;
    }

    glBindTexture(GL_TEXTURE_2D, texture_id);
    glTexImage2D(GL_TEXTURE_2D,
                 0, GL_RGBA, frame_cache->width / 2, frame_cache->height,
                 0, GL_RGBA, GL_UNSIGNED_BYTE, frame_cache->data);
    glBindTexture(GL_TEXTURE_2D, 0);

    uvc_free_frame(frame_cache);
    frame_cache = nullptr;
    
  } else {
    GM_RUNONCE(GM_ERR("UvcTexture", "Unsupported format " << formatToString(frame_cache->frame_format) << " to make texture of unconverted - consider setting ConvertToRgb to true."));
  }

}

bool UvcTexture::Impl::initialize_context() {
  uvc_error_t res = uvc_init(&context, NULL);
  if (res < 0) {
    closeAll();

    GM_ERR("UvcTexture", "Cannot initialize UVC context: " << uvc_strerror(res));
    return false;
  }
  GM_DBG1("UvcTexture", "UVC context successfully initialized");
  return true;
}

bool UvcTexture::Impl::locate_device(int vendor, int product, std::string serial) {
  const char *serial_cstr = serial.size() == 0 ? NULL : &serial[0];
  uvc_error_t res = uvc_find_device(context, &device, vendor, product, serial_cstr);
  if (res < 0) {
    closeAll();

    GM_ERR("UvcTexture", "Cannot find UVC device (vendor=" << vendor << ", product=" << product << ", serial=" << serial << "): " << uvc_strerror(res));
    return false;
  }
  GM_DBG1("UvcTexture", "Found UVC device (vendor=" << vendor << ", product=" << product << ", serial=" << serial << ")");
  return true;
}

bool UvcTexture::Impl::open_device() {
  /* Try to open the device: requires exclusive access */
  uvc_error_t res = uvc_open(device, &device_handle);
  if (res < 0) {
    closeAll();

    GM_ERR("UvcTexture", "Unable to open UVC device: " << uvc_strerror(res));
    return false;
  }
  GM_DBG1("UvcTexture", "Opened UVC device");
  return true;
}

bool UvcTexture::Impl::negotiate_format() {

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

    GM_ERR("UvcTexture", "Unable to negotiate UVC format: " << uvc_strerror(res));
    return false;
  }
  GM_DBG1("UvcTexture", "Successful UVC format negotiation");
  return true;
}

bool UvcTexture::Impl::start_streaming() {
  uvc_error_t res = uvc_start_streaming(device_handle, &stream_control, &UvcTexture::Impl::uvc_frame_cb, this, 0);
  if (res < 0) {
    closeAll();

    uvc_perror(res, "start_streaming"); /* unable to start stream */
    throw std::runtime_error(GM_STR("Unable to start stream: "
                                    << uvc_strerror(res)));
  }
  GM_DBG1("UvcTexture", "Started UVC streaming");
  return true;
}

/* This callback function runs once per frame. Use it to perform any
 * quick processing you need, or have it put the frame into your application's
 * input queue. If this function takes too long, you'll start losing frames. */
void UvcTexture::Impl::uvc_frame_cb(uvc_frame_t *frame, void *ptr) {

  GM_DBG3("UvcTexture",
           "Incoming UVC frame: " <<
           frame->width << "x" << frame->height
           << " in " << formatToString(frame->frame_format));

  UvcTexture::Impl *_impl = static_cast<UvcTexture::Impl*>(ptr);

  std::lock_guard<std::mutex> guard(_impl->data_lock);

  if (_impl->frame_cache)
    uvc_free_frame(_impl->frame_cache);

  _impl->frame_cache = uvc_allocate_frame(frame->data_bytes);
  uvc_duplicate_frame(frame, _impl->frame_cache);
}

void UvcTexture::Impl::closeAll() {

  std::lock_guard<std::mutex> guard(data_lock);

  if (frame_cache)
    uvc_free_frame(frame_cache);
  frame_cache = nullptr;

  if (device_handle != nullptr) {
    uvc_stop_streaming(device_handle);
    uvc_close(device_handle);
  }
  device_handle = nullptr;

  if (device != nullptr)
    uvc_unref_device(device);
  device = nullptr;

  if (context != nullptr)
    uvc_exit(context);
  context = nullptr;

  glDeleteTextures(1, &texture_id);
  texture_id = 0;
}

void UvcTexture::setResolution(gmCore::size2 res) {
  _impl->resolution = res;
}

void UvcTexture::setFramerate(int fps) {
  _impl->framerate = fps;
}

void UvcTexture::setFormat(std::string fmt) {
  _impl->format = UvcTexture::Impl::formatFromString(fmt);
}

void UvcTexture::setConvertToRbg(bool on) {
  _impl->convert_to_rgb = on;
}

uvc_frame_format UvcTexture::Impl::formatFromString(std::string s) {

#define FORMAT(A,B)                                                     \
  if (std::equal(s.begin(), s.end(),                                    \
                 #B, #B + strlen(#B),                                   \
                 [](unsigned char a, unsigned char b) {                 \
                   return std::tolower(a) == std::tolower(b);           \
                 }))                                                    \
    return A;

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

  GM_WRN("UvcTexture", "Unrecognized frame format " << s << "; using any");
  return UVC_FRAME_FORMAT_ANY;

#undef FORMAT
}

std::string UvcTexture::Impl::formatToString(uvc_frame_format f) {

#define FORMAT(A,B)                             \
  if (f == A) return std::string(#B); //

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

  return "invalid";

#undef FORMAT
}

END_NAMESPACE_GMGRAPHICS;

#endif
