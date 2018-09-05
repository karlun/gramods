
#include <gmVideo/UvcTexture.hh>

#ifdef gramods_ENABLE_libuvc

#include <libuvc/libuvc.h>
#include <stdio.h>
#include <thread>
#include <sstream>

BEGIN_NAMESPACE_GMVIDEO;

namespace {

  /* This callback function runs once per frame. Use it to perform any
   * quick processing you need, or have it put the frame into your application's
   * input queue. If this function takes too long, you'll start losing frames. */
  void uvc_frame_cb(uvc_frame_t *frame, void *ptr) {

    UvcTexture *_this = static_cast<UvcTexture*>(ptr);

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

    uvc_free_frame(bgr);
  }

}

struct UvcTexture::_UvcTexture {

  _UvcTexture(UvcTexture *ptr);
  ~_UvcTexture();

  void initialize_context();
  void locate_device(int vendor, int product, std::string serial);
  void open_device();
  void negotiate_format();
  void start_streaming();

  void startAll(int vendor, int product, std::string serial);
  void closeAll();

  UvcTexture *parent;

  uvc_context_t *context;
  uvc_device_t *device;
  uvc_device_handle_t *device_handle;
  uvc_stream_ctrl_t stream_control;
};

UvcTexture::UvcTexture(int vendor, int product, std::string serial)
  : _this(new _UvcTexture(this)) {
  _this->startAll(vendor, product, serial);
}

UvcTexture::_UvcTexture::_UvcTexture(UvcTexture *ptr)
  : parent(ptr),
    context(nullptr),
    device(nullptr),
    device_handle(nullptr) {}

UvcTexture::_UvcTexture::~_UvcTexture() {
  closeAll();
}

void UvcTexture::_UvcTexture::startAll(int vendor, int product, std::string serial) {
  initialize_context();
  locate_device(vendor, product, serial);
  open_device();
  negotiate_format();
  start_streaming();
}

void UvcTexture::_UvcTexture::initialize_context() {
  /* Initialize a UVC service context. Libuvc will set up its own libusb
   * context. Replace NULL with a libusb_context pointer to run libuvc
   * from an existing libusb context. */
  uvc_error_t res = uvc_init(&context, NULL);
  if (res < 0) {
    uvc_perror(res, "uvc_init");
    std::stringstream msg;
    msg << "cannot initialize a UVC service context: "
        << uvc_strerror(res);
    throw std::runtime_error(msg.str());
  }
  puts("UVC initialized");
}

void UvcTexture::_UvcTexture::locate_device(int vendor, int product, std::string serial) {
  const char *serial_cstr = serial.size() == 0 ? NULL : &serial[0];
  uvc_error_t res = uvc_find_device(context, &device, vendor, product, serial_cstr);
  if (res < 0) {
    closeAll();

    uvc_perror(res, "uvc_find_device"); /* no devices found */
    std::stringstream msg;
    msg << "cannot locate UVC device: "
        << uvc_strerror(res);
    throw std::runtime_error(msg.str());
  }
  puts("Device found");
}

void UvcTexture::_UvcTexture::open_device() {
  /* Try to open the device: requires exclusive access */
  uvc_error_t res = uvc_open(device, &device_handle);
  if (res < 0) {
    closeAll();

    uvc_perror(res, "uvc_open"); /* unable to open device */
    std::stringstream msg;
    msg << "cannot open UVC device: "
        << uvc_strerror(res);
    throw std::runtime_error(msg.str());
  }
  puts("Device opened");
}

void UvcTexture::_UvcTexture::negotiate_format() {
  /* Print out a message containing all the information that libuvc
   * knows about the device */
  uvc_print_diag(device_handle, stderr);
  /* Try to negotiate a 640x480 30 fps YUYV stream profile */
  uvc_error_t res = uvc_get_stream_ctrl_format_size
    (device_handle, &stream_control, /* result stored in ctrl */
     UVC_FRAME_FORMAT_YUYV, /* YUV 422, aka YUV 4:2:2. try _COMPRESSED */
     640, 480, 30 /* width, height, fps */
     );
  /* Print out the result */
  uvc_print_stream_ctrl(&stream_control, stderr);
  if (res < 0) {
    closeAll();

    uvc_perror(res, "get_mode"); /* device doesn't provide a matching stream */
    std::stringstream msg;
    msg << "No available stream matching requested format: "
        << uvc_strerror(res);
    throw std::runtime_error(msg.str());
  }
}

void UvcTexture::_UvcTexture::start_streaming() {
  uvc_error_t res = uvc_start_streaming(device_handle, &stream_control, uvc_frame_cb, parent, 0);
  if (res < 0) {
    closeAll();

    uvc_perror(res, "start_streaming"); /* unable to start stream */
    std::stringstream msg;
    msg << "Unable to start stream: "
        << uvc_strerror(res);
    throw std::runtime_error(msg.str());
  }
  puts("Streaming...");
}

void UvcTexture::_UvcTexture::closeAll() {
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
}

END_NAMESPACE_GMVIDEO;

#endif
