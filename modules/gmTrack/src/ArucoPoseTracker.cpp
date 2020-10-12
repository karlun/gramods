
#include <gmTrack/ArucoPoseTracker.hh>

#ifdef gramods_ENABLE_aruco

#include <gmCore/Console.hh>
#include <gmCore/RunLimited.hh>
#include <gmCore/RunOnce.hh>

#include <opencv2/aruco.hpp>

BEGIN_NAMESPACE_GMTRACK;

GM_OFI_DEFINE(ArucoPoseTracker);
GM_OFI_PARAM(ArucoPoseTracker, cameraConfigurationFile, std::filesystem::path, ArucoPoseTracker::setCameraConfigurationFile);
GM_OFI_PARAM(ArucoPoseTracker, inverted, bool, ArucoPoseTracker::setInverted);
GM_OFI_PARAM(ArucoPoseTracker, refindMarkers, bool, ArucoPoseTracker::setRefindMarkers);
GM_OFI_POINTER(ArucoPoseTracker, arucoBoard, gmTrack::ArucoBoard, ArucoPoseTracker::addArucoBoard);
GM_OFI_POINTER(ArucoPoseTracker, videoSource, gmTrack::OpenCvVideoSource, ArucoPoseTracker::setVideoSource);
GM_OFI_PARAM(ArucoPoseTracker, showDebug, bool, ArucoPoseTracker::setShowDebug);

struct ArucoPoseTracker::Impl {

  void update(gmCore::Updateable::clock::time_point t);
  bool getPose(std::map<int, PoseSample> &p);

  static bool readCameraParameters(std::filesystem::path filename,
                                   cv::Mat &camMatrix, cv::Mat &distCoeffs,
                                   int &width, int &height);

  std::vector<std::shared_ptr<ArucoBoard>> boards;
  std::shared_ptr<OpenCvVideoSource> video_source;

  int camera_width;
  int camera_height;
  cv::Mat camMatrix;
  cv::Mat distCoeffs;

  bool have_pose = false;
  std::map<int, PoseSample> samples;

  bool inverted = false;
  bool refind_markers = false;

  bool show_debug_output = false;
};

ArucoPoseTracker::ArucoPoseTracker()
  : _impl(std::make_unique<Impl>()) {}

void ArucoPoseTracker::update(gmCore::Updateable::clock::time_point t) {
  _impl->update(t);
}

void ArucoPoseTracker::addArucoBoard(std::shared_ptr<ArucoBoard> board) {
  _impl->boards.push_back(board);
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

bool ArucoPoseTracker::getPose(std::map<int, PoseSample> &p) {
  return _impl->getPose(p);
}

void ArucoPoseTracker::Impl::update(gmCore::Updateable::clock::time_point time_now) {

  if (boards.empty()) {
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

  cv::Mat image;
  if (!video_source->retrieve(image)) {
    GM_RUNLIMITED(GM_WRN("ArucoPoseTracker", "Video source did not provide image."), 1);
    have_pose = false;
    return;
  }

  cv::Mat debug_image;
  if (show_debug_output)
    image.copyTo(debug_image);

  if (camera_width != image.cols ||
      camera_height != image.rows) {

    if (camera_width * image.rows - camera_height * image.cols)
      GM_WRN("ArucoPoseTracker", "Video source image size (" << image.cols << "x" << image.rows << ") does not match camera parameters (" << camera_width << "x" << camera_height << ") - adjusting camera matrix accordingly. Even ratio differs, so result will be less than optimal.");
    else
      GM_WRN("ArucoPoseTracker", "Video source image size (" << image.cols << "x" << image.rows << ") does not match camera parameters (" << camera_width << "x" << camera_height << ") - adjusting camera matrix accordingly. The ratio is the same, but result may be less than optimal.");

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

  cv::Ptr<cv::aruco::DetectorParameters> detectorParams =
    cv::aruco::DetectorParameters::create();

  have_pose = false;

  for (int idx = 0; idx < boards.size(); ++idx) {

    auto board = this->boards[idx]->getBoard();

    if (!board) {
      GM_RUNLIMITED(GM_WRN("ArucoPoseTracker", "Board " << idx << " did not return aruco board."), 1);
      continue;
    }

    std::vector<int> ids;
    std::vector<std::vector<cv::Point2f>> corners, rejected;
    cv::Vec3d rvec, tvec;

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
        cv::aruco::estimatePoseBoard(corners, ids, board,
                                     camMatrix, distCoeffs,
                                     rvec, tvec);
    }

    if (markersOfBoardDetected) {

      cv::Matx33d rotm;
      cv::Rodrigues(rvec, rotm);
      Eigen::Map<Eigen::Matrix3d> R(cv::Mat(rotm).ptr<double>());
      Eigen::Quaterniond Q(R);

      if (inverted) {
        samples[idx].orientation = Eigen::Quaternionf(Q.conjugate());
        samples[idx].position = (samples[idx].orientation *
                                 Eigen::Vector3f(-tvec[0], -tvec[1], -tvec[2]));
      } else {
        samples[idx].orientation = Eigen::Quaternionf(Q);
        samples[idx].position = Eigen::Vector3f(tvec[0], tvec[1], tvec[2]);
      }
      samples[idx].time = time_now;

      have_pose = true;
    }

    if (show_debug_output) {

      if(ids.size() > 0)
        cv::aruco::drawDetectedMarkers(debug_image, corners, ids);

      if(rejected.size() > 0)
        cv::aruco::drawDetectedMarkers(debug_image, rejected, cv::noArray(), cv::Scalar(0, 0, 100));

      if(markersOfBoardDetected > 0) {
        cv::aruco::drawAxis(debug_image, camMatrix, distCoeffs, rvec, tvec, 0.1);

        std::vector<std::vector<cv::Point2f>> imagePoints;
        for (auto mpts : board->objPoints) {
          std::vector<cv::Point2f> imgpts;
          cv::projectPoints(mpts, rvec, tvec, camMatrix, distCoeffs, imgpts);
          imagePoints.push_back(imgpts);
        }
        cv::aruco::drawDetectedMarkers(debug_image, imagePoints, cv::noArray(), cv::Scalar(255, 0, 0));
      }
    }
  }

  if (show_debug_output) {
    cv::imshow("ArucoPoseTracker", debug_image);
    cv::waitKey(1);
  }
}

bool ArucoPoseTracker::Impl::getPose(std::map<int, PoseSample> &p) {
  if (!have_pose) return false;

  p = samples;
  return true;
}

void ArucoPoseTracker::setCameraConfigurationFile(std::filesystem::path file) {
  if (!Impl::readCameraParameters(file,
                                  _impl->camMatrix, _impl->distCoeffs,
                                  _impl->camera_width, _impl->camera_height))
    GM_WRN("ArucoPoseTracker", "Could not read camera configuration file '" << file << "'");
}

bool ArucoPoseTracker::Impl::readCameraParameters
(std::filesystem::path filename, cv::Mat &camMatrix, cv::Mat &distCoeffs, int &width, int &height) {

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
