
#include <gmTrack/ArucoPoseTracker.hh>

#ifdef gramods_ENABLE_aruco

#include <gmCore/RunLimited.hh>
#include <gmCore/RunOnce.hh>

#include <opencv2/aruco.hpp>

BEGIN_NAMESPACE_GMTRACK;

GM_OFI_DEFINE(ArucoPoseTracker);
GM_OFI_PARAM(ArucoPoseTracker, cameraConfigurationFile, std::string, ArucoPoseTracker::setCameraConfigurationFile);
GM_OFI_PARAM(ArucoPoseTracker, inverted, bool, ArucoPoseTracker::setInverted);
GM_OFI_PARAM(ArucoPoseTracker, refindMarkers, bool, ArucoPoseTracker::setRefindMarkers);
GM_OFI_POINTER(ArucoPoseTracker, arucoBoard, gmTrack::ArucoBoard, ArucoPoseTracker::setArucoBoard);
GM_OFI_POINTER(ArucoPoseTracker, videoSource, gmTrack::OpenCvVideoSource, ArucoPoseTracker::setVideoSource);
GM_OFI_PARAM(ArucoPoseTracker, showDebug, bool, ArucoPoseTracker::setShowDebug);

struct ArucoPoseTracker::Impl {

  void update(gmCore::Updateable::clock::time_point t);
  bool getPose(PoseSample &p);

  static bool readCameraParameters(std::string filename,
                                   cv::Mat &camMatrix, cv::Mat &distCoeffs,
                                   int &width, int &height);

  std::shared_ptr<ArucoBoard> board;
  std::shared_ptr<OpenCvVideoSource> video_source;

  int camera_width;
  int camera_height;
  cv::Mat camMatrix;
  cv::Mat distCoeffs;

  bool have_pose = false;
  PoseSample sample;

  bool inverted = false;
  bool refind_markers = false;

  bool show_debug_output = false;
};

ArucoPoseTracker::ArucoPoseTracker()
  : _impl(std::make_unique<Impl>()) {}

void ArucoPoseTracker::update(gmCore::Updateable::clock::time_point t) {
  _impl->update(t);
}

void ArucoPoseTracker::setArucoBoard(std::shared_ptr<ArucoBoard> board) {
  _impl->board = board;
}

void ArucoPoseTracker::setVideoSource(std::shared_ptr<OpenCvVideoSource> vs) {
  _impl->video_source = vs;
}

void ArucoPoseTracker::setInverted(bool on) {
  _impl->inverted = on;
}

void ArucoPoseTracker::setShowDebug(bool on) {
  _impl->show_debug_output = on;
}

void ArucoPoseTracker::setRefindMarkers(bool on) {
  _impl->refind_markers = on;
}

bool ArucoPoseTracker::getPose(PoseSample &p) {
  return _impl->getPose(p);
}

void ArucoPoseTracker::Impl::update(gmCore::Updateable::clock::time_point t) {

  if (!board) {
    GM_RUNONCE(GM_ERR("ArucoPoseTracker", "No board to track."));
    have_pose = false;
    return;
  }

  if (!video_source) {
    GM_RUNONCE(GM_ERR("ArucoPoseTracker", "No video source."));
    have_pose = false;
    return;
  }

  if (!camMatrix.total()) {
    GM_RUNONCE(GM_ERR("ArucoPoseTracker", "Camera parameters not set."));
    have_pose = false;
    return;
  }

  auto board = this->board->getBoard();

  if (!board) {
    GM_RUNLIMITED(GM_WRN("ArucoPoseTracker", "Board did not return aruco board."), 1);
    have_pose = false;
    return;
  }

  cv::Mat image;
  if (!video_source->retrieve(image)) {
    GM_RUNLIMITED(GM_WRN("ArucoPoseTracker", "Video source did not provide image."), 1);
    have_pose = false;
    return;
  }

  if (camera_width != image.cols ||
      camera_height != image.rows) {

    GM_WRN("ArucoPoseTracker", "Video source image size (" << image.cols << "x" << image.rows << ") did not match camera parameters (" << camera_width << "x" << camera_height << ") - adjusting camera matrix accordingly. The result will be less than optimal.");

    int new_width = image.cols;
    int new_height = image.rows;

    GM_INF("ArucoPoseTracker", "Provided camera matrix: " << camMatrix);
    camMatrix.at<double>(0,0) *= new_width / camera_width;
    camMatrix.at<double>(1,1) *= new_height / camera_height;
    camMatrix.at<double>(0,2) *= new_width / camera_width;
    camMatrix.at<double>(1,2) *= new_height / camera_height;
    GM_INF("ArucoPoseTracker", "New estimate of camera matrix: " << camMatrix);

    camera_width = new_width;
    camera_height = new_height;
  }

  std::vector<int> ids;
  std::vector<std::vector<cv::Point2f>> corners, rejected;
  cv::Vec3d rvec, tvec;

  cv::Ptr<cv::aruco::DetectorParameters> detectorParams =
    cv::aruco::DetectorParameters::create();

  // detect markers
  cv::aruco::detectMarkers(image, board->dictionary, corners, ids, detectorParams, rejected);

  // refind strategy to detect more markers
  if(refind_markers)
    cv::aruco::refineDetectedMarkers(image, board, corners, ids, rejected,
                                     camMatrix, distCoeffs);

  // estimate board pose
  int markersOfBoardDetected = 0;
  if(ids.size() > 0) {
    markersOfBoardDetected =
      cv::aruco::estimatePoseBoard(corners, ids, board, camMatrix, distCoeffs, rvec, tvec);
  }

  if (markersOfBoardDetected) {

    cv::Matx33d rotm;
    cv::Rodrigues(rvec, rotm);
    Eigen::Map<Eigen::Matrix3f> R(cv::Mat(rotm).ptr<float>());

    sample.orientation = Eigen::Quaternionf(R);
    sample.position = Eigen::Vector3f(tvec[0], tvec[1], tvec[2]);

    have_pose = true;
  } else {
    have_pose = false;
  }


  if (show_debug_output) {

    cv::Mat imageCopy;
    image.copyTo(imageCopy);

    if(ids.size() > 0)
      cv::aruco::drawDetectedMarkers(imageCopy, corners, ids);

    if(rejected.size() > 0)
      cv::aruco::drawDetectedMarkers(imageCopy, rejected, cv::noArray(), cv::Scalar(0, 0, 100));

    if(markersOfBoardDetected > 0) {
      cv::aruco::drawAxis(imageCopy, camMatrix, distCoeffs, rvec, tvec, 0.1);

      std::vector<std::vector<cv::Point2f>> imagePoints;
      for (auto mpts : board->objPoints) {
        std::vector<cv::Point2f> imgpts;
        cv::projectPoints(mpts, rvec, tvec, camMatrix, distCoeffs, imgpts);
        imagePoints.push_back(imgpts);
      }
      cv::aruco::drawDetectedMarkers(imageCopy, imagePoints, cv::noArray(), cv::Scalar(255, 0, 0));
    }

    cv::imshow("out", imageCopy);
    cv::waitKey(1);
  }
}

bool ArucoPoseTracker::Impl::getPose(PoseSample &p) {
  if (!have_pose) return false;

  p = sample;
  return true;
}

void ArucoPoseTracker::setCameraConfigurationFile(std::string file) {
  if (!Impl::readCameraParameters(file,
                                  _impl->camMatrix, _impl->distCoeffs,
                                  _impl->camera_width, _impl->camera_height))
    GM_WRN("ArucoPoseTracker", "Could not read camera configuration file '" << file << "'");
}

bool ArucoPoseTracker::Impl::readCameraParameters
(std::string filename, cv::Mat &camMatrix, cv::Mat &distCoeffs, int &width, int &height) {

  cv::FileStorage fs(filename, cv::FileStorage::READ);
  if(!fs.isOpened()) return false;

  fs["image_width"] >> width;
  fs["image_height"] >> height;
  fs["camera_matrix"] >> camMatrix;
  fs["distortion_coefficients"] >> distCoeffs;
  GM_INF("ArucoPoseTracker", "Read camera matrix: " << camMatrix.total() << " elements.");
  GM_INF("ArucoPoseTracker", "Read camera distortion: " << distCoeffs.total() << " elements.");

  return true;
}

END_NAMESPACE_GMTRACK;

#endif
