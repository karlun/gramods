
#include <gmTrack/OpenCvVideoCapture.hh>

#ifdef gramods_ENABLE_aruco

BEGIN_NAMESPACE_GMTRACK;

GM_OFI_DEFINE(OpenCvVideoCapture);
GM_OFI_PARAM(OpenCvVideoCapture, videoFile, std::string, OpenCvVideoCapture::setVideoFile);
GM_OFI_PARAM(OpenCvVideoCapture, cameraId, int, OpenCvVideoCapture::setCameraId);

struct OpenCvVideoCapture::Impl {

  void update(gmCore::Updateable::clock::time_point t);
  bool retrieve(cv::Mat &image);

  cv::VideoCapture video_capture;
  bool alive = false;
};

OpenCvVideoCapture::OpenCvVideoCapture()
  : Updateable(20),
    _impl(std::make_unique<Impl>()) {}

void OpenCvVideoCapture::setVideoFile(std::string file) {
  _impl->video_capture.open(file);
}

void OpenCvVideoCapture::setCameraId(int id) {
  _impl->video_capture.open(id);
}

void OpenCvVideoCapture::update(gmCore::Updateable::clock::time_point t) {
  _impl->update(t);
}

void OpenCvVideoCapture::Impl::update(gmCore::Updateable::clock::time_point t) {
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
