
#include <gmTrack/OpenCvVideoCapture.hh>

#ifdef gramods_ENABLE_OpenCV

#include <gmCore/Console.hh>
#include <gmCore/FileResolver.hh>

BEGIN_NAMESPACE_GMTRACK;

GM_OFI_DEFINE(OpenCvVideoCapture);
GM_OFI_PARAM2(OpenCvVideoCapture, videoFile, std::filesystem::path, setVideoFile);
GM_OFI_PARAM2(OpenCvVideoCapture, cameraId, int, setCameraId);
GM_OFI_PARAM2(OpenCvVideoCapture, cameraWidth, int, setCameraWidth);
GM_OFI_PARAM2(OpenCvVideoCapture, cameraHeight, int, setCameraHeight);
GM_OFI_PARAM2(OpenCvVideoCapture, cameraFramerate, int, setCameraFramerate);
GM_OFI_PARAM2(OpenCvVideoCapture, cameraFourCC, std::string, setCameraFourCC);
GM_OFI_PARAM2(OpenCvVideoCapture, backend, std::string, setBackend);

struct OpenCvVideoCapture::Impl {

  void update();
  bool retrieve(cv::Mat &image);

  void openCamera(int id);
  void openVideo(std::filesystem::path file);

  static cv::VideoCaptureAPIs backendFromString(std::string api);

  std::filesystem::path video_file;
  int camera_id = 0;

  int width = 0;
  int height = 0;
  int framerate = 0;
  int fourcc = 0;
  cv::VideoCaptureAPIs backend = backendFromString("ANY");

  bool use_camera = true;

  cv::VideoCapture video_capture;
  bool alive = false;

  bool initialized = false;

};

OpenCvVideoCapture::OpenCvVideoCapture()
  : Updateable(20),
    _impl(std::make_unique<Impl>()) {}

void OpenCvVideoCapture::setVideoFile(std::filesystem::path file) {
  _impl->use_camera = false;
  _impl->video_file = gmCore::FileResolver::getDefault()->resolve(
      file, gmCore::FileResolver::Check::ReadableFile);
  _impl->initialized = false;
}

void OpenCvVideoCapture::setCameraId(int id) {
  _impl->use_camera = true;
  _impl->camera_id = id;
  _impl->initialized = false;
}

void OpenCvVideoCapture::setCameraHeight(int H) {
  _impl->height = H;
}

void OpenCvVideoCapture::setCameraWidth(int W) {
  _impl->width = W;
}

void OpenCvVideoCapture::setCameraFramerate(int R) {
  _impl->framerate = R;
}

namespace {
  std::string fourccIntToString(int cc) {
    std::stringstream ss;
    ss << (char)((cc >>  0) & 0xff);
    ss << (char)((cc >>  8) & 0xff);
    ss << (char)((cc >> 16) & 0xff);
    ss << (char)((cc >> 24) & 0xff);
    return ss.str();
  }

  int fourccIntFromString(std::string s) {

    if (s.length() != 4) {
      GM_WRN("OpenCvVideoCapture", "String '" << s << "' is not a FourCC - reverting to 0 (zero)");
      return 0;
    }

    return ((s[0]      ) |
            (s[1] <<  8) |
            (s[2] << 16) |
            (s[3] << 24));
  }

}

void OpenCvVideoCapture::setCameraFourCC(std::string cc) {
  _impl->fourcc = fourccIntFromString(cc);
}

void OpenCvVideoCapture::setBackend(std::string b) {
  _impl->backend = Impl::backendFromString(b);
}

cv::VideoCaptureAPIs OpenCvVideoCapture::Impl::backendFromString(std::string api) {

#define BACKEND(NAME) if (api == #NAME) return cv::CAP_##NAME;

  BACKEND(ANY);
  BACKEND(V4L);
  BACKEND(V4L2);
  BACKEND(FIREWIRE);
  BACKEND(FIREWARE);
  BACKEND(IEEE1394);
  BACKEND(DC1394);
  BACKEND(CMU1394);
  BACKEND(DSHOW);
  BACKEND(PVAPI);
  BACKEND(OPENNI);
  BACKEND(OPENNI_ASUS);
  BACKEND(ANDROID);
  BACKEND(XIAPI);
  BACKEND(AVFOUNDATION);
  BACKEND(GIGANETIX);
  BACKEND(MSMF);
  BACKEND(WINRT);
  BACKEND(INTELPERC);
  BACKEND(REALSENSE);
  BACKEND(OPENNI2);
  BACKEND(OPENNI2_ASUS);
  BACKEND(GPHOTO2);
  BACKEND(GSTREAMER);
  BACKEND(FFMPEG);
  BACKEND(IMAGES);
  BACKEND(ARAVIS);
  BACKEND(OPENCV_MJPEG);
  BACKEND(INTEL_MFX);
  BACKEND(XINE);

  GM_WRN("OpenCvVideoCapture", "Not a valid backend API: '" << api << "' - reverting to 'ANY'");
  return cv::CAP_ANY;

#undef BACKEND

}

void OpenCvVideoCapture::Impl::openVideo(std::filesystem::path file) {

  if (video_capture.open(file.string(), backend)){

    GM_DBG1("OpenCvVideoCapture", "Opened video file '" << file << "'");

    GM_DBG2("OpenCvVideoCapture", "Backend: " << video_capture.getBackendName() << ".");
    GM_DBG2("OpenCvVideoCapture",
            "Format: " << video_capture.get(cv::CAP_PROP_FRAME_WIDTH) << " x "
                       << video_capture.get(cv::CAP_PROP_FRAME_HEIGHT) << " @"
                       << video_capture.get(cv::CAP_PROP_FPS) << " ("
                       << fourccIntToString(
                              int(video_capture.get(cv::CAP_PROP_FOURCC)))
                       << ").");
  } else {
    GM_ERR("OpenCvVideoCapture", "Could not open video file '" << file << "'");
  }
}

void OpenCvVideoCapture::Impl::openCamera(int id) {

  if (video_capture.open(id, backend)) {

    GM_DBG1("OpenCvVideoCapture", "Opened camera with id " << id << ".");

    if (width > 0) video_capture.set(cv::CAP_PROP_FRAME_WIDTH, width);
    if (height > 0) video_capture.set(cv::CAP_PROP_FRAME_HEIGHT, height);
    if (framerate > 0) video_capture.set(cv::CAP_PROP_FPS, framerate);
    if (fourcc > 0) video_capture.set(cv::CAP_PROP_FOURCC, fourcc);

    GM_DBG2("OpenCvVideoCapture", "Backend: " << video_capture.getBackendName() << ".");
    GM_DBG2("OpenCvVideoCapture",
            "Format: " << video_capture.get(cv::CAP_PROP_FRAME_WIDTH) << " x "
                       << video_capture.get(cv::CAP_PROP_FRAME_HEIGHT) << " @"
                       << video_capture.get(cv::CAP_PROP_FPS) << " ("
                       << fourccIntToString(
                              int(video_capture.get(cv::CAP_PROP_FOURCC)))
                       << ").");
  } else {
    GM_ERR("OpenCvVideoCapture", "Could not open camera with id " << id << ".");
  }
}

void OpenCvVideoCapture::update(gmCore::Updateable::clock::time_point, size_t) {
  _impl->update();
}

void OpenCvVideoCapture::Impl::update() {

  if (!initialized) {

    if (use_camera) openCamera(camera_id);
    else openVideo(video_file);

    initialized = true;
  }

  alive = video_capture.grab();
}

bool OpenCvVideoCapture::retrieve(cv::Mat &image) {
  return _impl->retrieve(image);
}

bool OpenCvVideoCapture::Impl::retrieve(cv::Mat &image) {
  if (!alive) return false;
  video_capture.retrieve(image);
  return true;
}

END_NAMESPACE_GMTRACK;

#endif
