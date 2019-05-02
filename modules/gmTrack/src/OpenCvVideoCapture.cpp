
#include <gmTrack/OpenCvVideoCapture.hh>

#ifdef gramods_ENABLE_aruco

BEGIN_NAMESPACE_GMTRACK;

GM_OFI_DEFINE(OpenCvVideoCapture);
GM_OFI_PARAM(OpenCvVideoCapture, videoFile, std::string, OpenCvVideoCapture::setVideoFile);
GM_OFI_PARAM(OpenCvVideoCapture, cameraId, int, OpenCvVideoCapture::setCameraId);

struct OpenCvVideoCapture::Impl {

  void update(gmCore::Updateable::clock::time_point t);
  bool retrieve(cv::Mat &image);

  void openCamera(int id);
  void openVideo(std::string file);

  std::string video_file;
  int camera_id = 0;
  bool use_camera = true;
  bool initialized = false;

  cv::VideoCapture video_capture;
  bool alive = false;
};

OpenCvVideoCapture::OpenCvVideoCapture()
  : Updateable(20),
    _impl(std::make_unique<Impl>()) {}

void OpenCvVideoCapture::setVideoFile(std::string file) {
  _impl->use_camera = false;
  _impl->video_file = file;
  _impl->initialized = false;
}

void OpenCvVideoCapture::setCameraId(int id) {
  _impl->use_camera = true;
  _impl->camera_id = id;
  _impl->initialized = false;
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
}

void OpenCvVideoCapture::Impl::openVideo(std::string file) {
  if (video_capture.open(file)){
    GM_INF("OpenCvVideoCapture", "Opened video file '" << file << "'");
    GM_VINF("OpenCvVideoCapture", "Backend: " << video_capture.getBackendName() << ".");
    GM_VINF("OpenCvVideoCapture", "Format: "
            << video_capture.get(cv::CAP_PROP_FRAME_WIDTH) << " x "
            << video_capture.get(cv::CAP_PROP_FRAME_HEIGHT)
            << " @" << video_capture.get(cv::CAP_PROP_FPS)
            << " (" << fourccIntToString(video_capture.get(cv::CAP_PROP_FOURCC)) << ").");
  } else {
    GM_ERR("OpenCvVideoCapture", "Could not open video file '" << file << "'");
  }
}

void OpenCvVideoCapture::Impl::openCamera(int id) {
  if (video_capture.open(id)) {
    GM_INF("OpenCvVideoCapture", "Opened camera with id " << id << ".");
    GM_VINF("OpenCvVideoCapture", "Backend: " << video_capture.getBackendName() << ".");
    GM_VINF("OpenCvVideoCapture", "Format: "
            << video_capture.get(cv::CAP_PROP_FRAME_WIDTH) << " x "
            << video_capture.get(cv::CAP_PROP_FRAME_HEIGHT)
            << " @" << video_capture.get(cv::CAP_PROP_FPS)
            << " (" << fourccIntToString(video_capture.get(cv::CAP_PROP_FOURCC)) << ").");
  } else {
    GM_ERR("OpenCvVideoCapture", "Could not open camera with id " << id << ".");
  }
}

void OpenCvVideoCapture::update(gmCore::Updateable::clock::time_point t) {
  _impl->update(t);
}

void OpenCvVideoCapture::Impl::update(gmCore::Updateable::clock::time_point t) {

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
