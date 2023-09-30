
#include <gmGraphics/UvcTexture.hh>

#ifdef gramods_ENABLE_libuvc

#include <gmCore/RunOnce.hh>
#include <gmCore/Console.hh>
#include <gmCore/StringFile.hh>

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
GM_OFI_PARAM2(UvcTexture, decode, bool, setDecode);


struct UvcTexture::Impl {

  ~Impl();

#ifdef gramods_ENABLE_OpenCV
  bool retrieve(cv::Mat &image);
#endif

  bool update_rgb_cache();

  static uvc_frame_format formatFromString(std::string s);
  static std::string formatToString(uvc_frame_format f);

  bool initialize_context();
  bool locate_device(int vendor, int product, std::string serial);
  bool open_device();
  bool negotiate_format();
  bool start_streaming();

  static void uvc_frame_cb(uvc_frame_t *frame, void *ptr);
  void uvc_frame_cb(uvc_frame_t *frame);

  void startAll(int vendor, int product, std::string serial);
  void closeAll();

  bool triggerStill(gmCore::size2 res);

  void update();
  GLuint getGLTextureID() { return texture_id; }

  uvc_context_t *context = nullptr;
  uvc_device_t *device = nullptr;
  uvc_device_handle_t *device_handle = nullptr;
  uvc_stream_ctrl_t stream_control;

  gmCore::size2 resolution = { 640, 480 };
  int framerate = 30;
  uvc_frame_format format = formatFromString("any");
  bool decode_to_rgb = true;

  uvc_frame_t *frm_cache = nullptr;
  uvc_frame_t *rgb_cache = nullptr;
  std::mutex data_lock;
  bool texture_up_to_date = false;

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

UvcTexture::Impl::~Impl() {
  closeAll();
}

GLuint UvcTexture::updateTexture(size_t frame_number, Eye eye) {
  if (!_impl->started) {
    _impl->startAll(vendor, product, serial);
    _impl->started = true;
  }

  _impl->update();
  return _impl->getGLTextureID();
}

#ifdef gramods_ENABLE_OpenCV

bool UvcTexture::retrieve(cv::Mat &image) {
  return _impl->retrieve(image);
}

bool UvcTexture::Impl::retrieve(cv::Mat &image) {

  if (!frm_cache) return false;

  if (decode_to_rgb) {
    if (!update_rgb_cache()) return false;
    image =
        cv::Mat(rgb_cache->height, rgb_cache->width, CV_8UC3, rgb_cache->data)
            .clone();
  } else {
    image =
        cv::Mat(
            frm_cache->height, frm_cache->width, CV_8UC3, frm_cache->data)
            .clone();
  }
  return true;
}

#endif

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

  if (texture_up_to_date) return;

  if (!frm_cache) return;

  if (decode_to_rgb) {

    GM_DBG2("UvcTexture", "Decoding frame cache to RGB.");
    if (!update_rgb_cache()) return;

    glBindTexture(GL_TEXTURE_2D, texture_id);
    glTexImage2D(GL_TEXTURE_2D,
                 0, GL_RGB, rgb_cache->width, rgb_cache->height,
                 0, GL_RGB, GL_UNSIGNED_BYTE, rgb_cache->data);
    glBindTexture(GL_TEXTURE_2D, 0);
    texture_up_to_date = true;

  } else if (frm_cache->frame_format == UVC_FRAME_FORMAT_YUYV ||
             frm_cache->frame_format == UVC_FRAME_FORMAT_UYVY) {
    // Two different formats with identical macropixel size

    if (frm_cache->data_bytes > frm_cache->width * frm_cache->height * 2) {
      GM_WRN("UvcTexture", "Too much data in frame");
    }

    if (frm_cache->data_bytes < frm_cache->width * frm_cache->height * 2) {
      GM_ERR("UvcTexture", "Too little data in frame");
      return;
    }

    glBindTexture(GL_TEXTURE_2D, texture_id);
    glTexImage2D(GL_TEXTURE_2D,
                 0, GL_RGBA, frm_cache->width / 2, frm_cache->height,
                 0, GL_RGBA, GL_UNSIGNED_BYTE, frm_cache->data);
    glBindTexture(GL_TEXTURE_2D, 0);
    texture_up_to_date = true;

    uvc_free_frame(frm_cache);
    frm_cache = nullptr;
    
  } else {
    GM_RUNONCE(GM_ERR("UvcTexture", "Unsupported format " << formatToString(frm_cache->frame_format) << " to make texture of unconverted - consider setting ConvertToRgb to true."));
  }

}

bool UvcTexture::Impl::update_rgb_cache() {

  if (rgb_cache) return true;

  rgb_cache = uvc_allocate_frame(frm_cache->width * frm_cache->height * 3);
  if (!rgb_cache) {
    GM_ERR("UvcTexture", "Unable to allocate rgb frame");
    return false;
  }

  uvc_error_t ret = frm_cache->frame_format == UVC_FRAME_FORMAT_MJPEG
                        ? uvc_mjpeg2rgb(frm_cache, rgb_cache)
                        : uvc_any2rgb(frm_cache, rgb_cache);
  if (ret) {
    GM_ERR("UvcTexture",
           "Cannot convert incoming data "
               << "(" << formatToString(frm_cache->frame_format)
               << "): " << uvc_strerror(ret));
    uvc_free_frame(rgb_cache);
    rgb_cache = nullptr;
    return false;
  }

  if (rgb_cache->width == 0 || rgb_cache->height == 0) {
    GM_ERR("UvcTexture", "Ignoring empty frame");
    uvc_free_frame(rgb_cache);
    rgb_cache = nullptr;
    return false;
  }

  if (rgb_cache->data_bytes > rgb_cache->width * rgb_cache->height * 3) {
    GM_WRN("UvcTexture", "Too much data in frame");
  }

  if (rgb_cache->data_bytes < rgb_cache->width * rgb_cache->height * 3) {
    GM_ERR("UvcTexture", "Too little data in frame");
    return false;
  }

  return true;
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
  gmCore::StringFile string_file;
  uvc_print_diag(device_handle, string_file.getFilePtr());
  GM_DBG2("UvcTexture", string_file.finalize());

  uvc_error_t res = uvc_get_stream_ctrl_format_size
    (device_handle, &stream_control,
     format, resolution[0], resolution[1], framerate);

  /* Print out the result */
  uvc_print_stream_ctrl(&stream_control, string_file.getFilePtr());
  GM_DBG2("UvcTexture", string_file.finalize());

  if (res < 0) {
    closeAll();

    GM_ERR("UvcTexture", "Unable to negotiate UVC format: " << uvc_strerror(res));
    return false;
  }
  GM_DBG1("UvcTexture", "Successful UVC format negotiation");
  return true;
}

bool UvcTexture::Impl::start_streaming() {
  uvc_error_t res = uvc_start_streaming(
      device_handle, &stream_control, &UvcTexture::Impl::uvc_frame_cb, this, 0);
  if (res < 0) {
    closeAll();

    GM_ERR("UvcTexture", "Unable to start stream: " << uvc_strerror(res));
    return false;
  }
  GM_DBG1("UvcTexture", "Started UVC streaming");
  return true;
}

/* This callback function runs once per frame. Use it to perform any
 * quick processing you need, or have it put the frame into your application's
 * input queue. If this function takes too long, you'll start losing frames. */
void UvcTexture::Impl::uvc_frame_cb(uvc_frame_t *frame, void *ptr) {
  static_cast<UvcTexture::Impl *>(ptr)->uvc_frame_cb(frame);
}

void UvcTexture::Impl::uvc_frame_cb(uvc_frame_t *frame) {

  GM_DBG3("UvcTexture",
           "Incoming UVC frame: " <<
           frame->width << "x" << frame->height
           << " in " << formatToString(frame->frame_format));

  std::lock_guard<std::mutex> guard(data_lock);

  if (frm_cache)
    uvc_free_frame(frm_cache);

  frm_cache = uvc_allocate_frame(frame->data_bytes);
  uvc_duplicate_frame(frame, frm_cache);

  texture_up_to_date = false;
}

void UvcTexture::Impl::closeAll() {

  std::lock_guard<std::mutex> guard(data_lock);

  if (frm_cache) uvc_free_frame(frm_cache);
  frm_cache = nullptr;

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

void UvcTexture::setDecode(bool on) {
  _impl->decode_to_rgb = on;
  _impl->texture_up_to_date = false;
}

bool UvcTexture::triggerStill(gmCore::size2 res) {
  return _impl->triggerStill(res);
}

bool UvcTexture::Impl::triggerStill(gmCore::size2 size) {
#ifndef gramods_ENABLE_libuvc_007
  GM_RUNONCE(GM_WRN("UvcTexture",
                    "Gramods built without support for UVC still image frame;"
                    " could not accomodate request!"));
  return false;
#else

  std::lock_guard<std::mutex> guard(data_lock);

  uvc_still_ctrl_t still_control;
  uvc_error_t res = uvc_get_still_ctrl_format_size(
      device_handle, &stream_control, &still_control, size[0], size[1]);

  if (res < 0) {
    GM_ERR("UvcTexture",
           "Could not find format for still image size "
               << size[0] << "x" << size[1] << "): " << uvc_strerror(res));
    return false;
  }

  res = uvc_trigger_still(device_handle, &still_control);

  if (res < 0) {
    GM_ERR("UvcTexture",
           "Could not trigger still image capture: " << uvc_strerror(res));
    return false;
  }

  GM_DBG2("UvcTexture",
          "Triggered still image capture (" << size[0] << "x" << size[1]
                                            << ")");
  return true;
#endif
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
