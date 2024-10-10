
#include <gmTrack/ArucoPoseTracker.hh>

#ifdef gramods_ENABLE_OpenCV_objdetect

#include <gmCore/Console.hh>
#include <gmCore/RunLimited.hh>
#include <gmCore/RunOnce.hh>
#include <gmCore/FileResolver.hh>

#include <opencv2/objdetect.hpp>

BEGIN_NAMESPACE_GMTRACK;

GM_OFI_DEFINE(ArucoPoseTracker);
GM_OFI_PARAM2(ArucoPoseTracker, cameraConfigurationFile, std::filesystem::path, setCameraConfigurationFile);
GM_OFI_PARAM2(ArucoPoseTracker, inverted, bool, setInverted);
GM_OFI_PARAM2(ArucoPoseTracker, refindMarkers, bool, setRefindMarkers);
GM_OFI_POINTER2(ArucoPoseTracker, arucoBoard, gmTrack::ArucoBoard, addArucoBoard);
GM_OFI_POINTER2(ArucoPoseTracker, videoSource, gmTrack::OpenCvVideoCapture, setVideoSource);
GM_OFI_PARAM2(ArucoPoseTracker, showDebug, bool, setShowDebug);

struct ArucoPoseTracker::Impl {

  void update(gmCore::Updateable::clock::time_point t);
  bool getPose(std::map<int, PoseSample> &p);

  static bool readCameraParameters(std::filesystem::path filename,
                                   cv::Mat &camMatrix, cv::Mat &distCoeffs,
                                   int &width, int &height);

  std::vector<std::shared_ptr<ArucoBoard>> boards;
  std::shared_ptr<OpenCvVideoCapture> video_source;

  int camera_width;
  int camera_height;
  cv::Mat camMatrix;
  cv::Mat distCoeffs;

  std::map<int, PoseSample> samples;

  bool inverted = false;
  bool refind_markers = false;

  bool show_debug_output = false;
};

ArucoPoseTracker::ArucoPoseTracker()
  : _impl(std::make_unique<Impl>()) {}

void ArucoPoseTracker::update(gmCore::Updateable::clock::time_point time,
                              size_t frame) {
  _impl->update(time);
}

void ArucoPoseTracker::addArucoBoard(std::shared_ptr<ArucoBoard> board) {
  _impl->boards.push_back(board);
}

void ArucoPoseTracker::setVideoSource(std::shared_ptr<OpenCvVideoCapture> vs) {
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
    samples.clear();
    return;
  }

  if (!video_source) {
    GM_RUNONCE(GM_ERR("ArucoPoseTracker", "No video source."));
    samples.clear();
    return;
  }

  if (!camMatrix.total()) {
    GM_RUNONCE(GM_ERR("ArucoPoseTracker", "Camera parameters not set."));
    samples.clear();
    return;
  }

  cv::Mat image;
  if (!video_source->retrieve(image)) {
    GM_RUNLIMITED(GM_WRN("ArucoPoseTracker", "Video source did not provide image."), 1);
    samples.clear();
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

    GM_DBG1("ArucoPoseTracker", "Provided camera matrix: " << camMatrix);
    camMatrix.at<double>(0, 0) *= double(new_width) / double(camera_width);
    camMatrix.at<double>(1, 1) *= double(new_height) / double(camera_height);
    camMatrix.at<double>(0, 2) *= double(new_width) / double(camera_width);
    camMatrix.at<double>(1, 2) *= double(new_height) / double(camera_height);
    GM_DBG1("ArucoPoseTracker", "New estimate of camera matrix: " << camMatrix);

    camera_width = new_width;
    camera_height = new_height;
  }

  cv::aruco::DetectorParameters detectorParams;
  cv::aruco::RefineParameters refineParams;

  struct DetectionCache {
    cv::aruco::Dictionary dictionary;
    std::vector<int> ids;
    std::vector<std::vector<cv::Point2f>> corners, rejected;
  };
  std::vector<DetectionCache> detection_cache;

  for (size_t idx = 0; idx < boards.size(); ++idx) {

    auto board = this->boards[idx]->getBoard();

    if (!board) {
      GM_RUNLIMITED(GM_WRN("ArucoPoseTracker", "Board " << idx << " did not return aruco board."), 1);
      continue;
    }

    std::vector<int> ids;
    std::vector<std::vector<cv::Point2f>> corners, rejected;

    auto dictionary = board->getDictionary();
    cv::aruco::ArucoDetector detector(
        dictionary, detectorParams, refineParams);

    auto find_cache = [dictionary](const DetectionCache &dc) {
      return std::equal(dictionary.bytesList.begin<uchar>(),
                        dictionary.bytesList.end<uchar>(),
                        dc.dictionary.bytesList.begin<uchar>(),
                        dc.dictionary.bytesList.end<uchar>()) &&
             (dictionary.markerSize == dc.dictionary.markerSize) &&
             (dictionary.maxCorrectionBits == dc.dictionary.maxCorrectionBits);
    };

    auto cache = std::find_if(
        detection_cache.begin(), detection_cache.end(), find_cache);
    if (cache != detection_cache.end()) {
      ids = cache->ids;
      corners = cache->corners;
      rejected = cache->rejected;
    } else {
      // detect markers
      detector.detectMarkers(image, corners, ids, rejected);

      if (ids.empty()) continue;

      detection_cache.push_back({dictionary, ids, corners, rejected});

      if (show_debug_output) {
        if (ids.size() > 0)
          cv::aruco::drawDetectedMarkers(debug_image, corners, ids);

        if (rejected.size() > 0)
          cv::aruco::drawDetectedMarkers(debug_image, rejected, cv::noArray(), cv::Scalar(0, 0, 100));
      }
    }

    // refind strategy to detect more markers
    if (refind_markers)
      detector.refineDetectedMarkers(
          image, *board, corners, ids, rejected, camMatrix, distCoeffs);

    cv::Mat objPoints, imgPoints;
    board->matchImagePoints(corners, ids, objPoints, imgPoints);

    if (objPoints.total() == 0) continue;

    // Find pose
    cv::Vec3d rvec, tvec;
    try {
      cv::solvePnP(objPoints, imgPoints, camMatrix, distCoeffs, rvec, tvec);
    } catch (const std::exception &e) {
      GM_ERR("ArucoPoseTracker", "solvePnP failed: " << e.what());
      continue;
    } catch (...) {
      GM_ERR("ArucoPoseTracker", "solvePnP failed of unknown reason");
      continue;
    }

    cv::Matx33d rotm;
    cv::Rodrigues(rvec, rotm);
    Eigen::Map<Eigen::Matrix3d> R(cv::Mat(rotm).ptr<double>());
    Eigen::Quaterniond Q(R);

    PoseSample sample;
    if (inverted) {
      sample.orientation = Eigen::Quaternionf(Q.conjugate());
      sample.position =
          (samples[idx].orientation *
           Eigen::Vector3d(-tvec[0], -tvec[1], -tvec[2]).cast<float>());
    } else {
      sample.orientation = Eigen::Quaternionf(Q);
      sample.position =
          Eigen::Vector3d(tvec[0], tvec[1], tvec[2]).cast<float>();
    }
    sample.time = time_now;

    samples[idx] = sample;

    if (show_debug_output) {
      std::vector<std::vector<cv::Point2f>> imagePoints;
      for (auto mpts : board->getObjPoints()) {
        std::vector<cv::Point2f> imgpts;
        cv::projectPoints(mpts, rvec, tvec, camMatrix, distCoeffs, imgpts);
        imagePoints.push_back(imgpts);
      }
      cv::aruco::drawDetectedMarkers(
          debug_image, imagePoints, cv::noArray(), cv::Scalar(255, 0, 0));

      // Draw frame axes with OpenGL axes convention
      rotm = rotm * cv::Matx33d(1, 0, 0, 0, -1, 0, 0, 0, -1);
      cv::Rodrigues(rotm, rvec);
      cv::drawFrameAxes(debug_image, camMatrix, distCoeffs, rvec, tvec, 0.1f);
    }
  }

  if (show_debug_output) {
    cv::imshow("ArucoPoseTracker", debug_image);
    cv::waitKey(1);
  }
}

bool ArucoPoseTracker::Impl::getPose(std::map<int, PoseSample> &p) {
  if (samples.empty()) return false;

  p = samples;
  return true;
}

void ArucoPoseTracker::setCameraConfigurationFile(std::filesystem::path file) {
  file = gmCore::FileResolver::getDefault()->resolve(
      file, gmCore::FileResolver::Check::ReadableFile);
  if (!Impl::readCameraParameters(file,
                                  _impl->camMatrix, _impl->distCoeffs,
                                  _impl->camera_width, _impl->camera_height))
    GM_WRN("ArucoPoseTracker", "Could not read camera configuration file '" << file << "'");
}

bool ArucoPoseTracker::Impl::readCameraParameters
(std::filesystem::path filename, cv::Mat &camMatrix, cv::Mat &distCoeffs, int &width, int &height) {

  cv::FileStorage fs(filename.string(), cv::FileStorage::READ);
  if(!fs.isOpened()) {
    GM_ERR("ArucoPoseTracker",
           "Could not open file '" << filename
                                   << "' for reading camera parameters!");
    return false;
  }

  if (!fs["image_width"].empty() && !fs["image_height"].empty()) {
    fs["image_width"] >> width;
    fs["image_height"] >> height;
  } else if (!fs["cameraResolution"].empty()) {
    cv::Size image_size;
    fs["cameraResolution"] >> image_size;
    width = image_size.width;
    height = image_size.height;
  } else {
    GM_ERR("ArucoPoseTracker",
           "Could not find camera resolution"
           " (cameraResolution or image_width and image_height)"
           " in configuration file '"
               << filename << "'!");
    return false;
  }

  if (!fs["camera_matrix"].empty()) {
    fs["camera_matrix"] >> camMatrix;
  } else if (!fs["cameraMatrix"].empty()) {
    fs["cameraMatrix"] >> camMatrix;
  } else {
    GM_ERR("ArucoPoseTracker",
           "Could not find camera matrix"
           " (camera_matrix or cameraMatrix)"
           " in configuration file '"
               << filename << "'!");
    return false;
  }

  if (!fs["distortion_coefficients"].empty()) {
    fs["distortion_coefficients"] >> distCoeffs;
  } else if (!fs["dist_coeffs"].empty()) {
    fs["dist_coeffs"] >> distCoeffs;
  } else {
    GM_WRN("ArucoPoseTracker",
           "Could not find distortion coefficients"
           " (distortion_coefficients or dist_coeffs)"
           " in configuration file '"
               << filename << "'!");
    distCoeffs = cv::Mat();
  }

  GM_DBG1("ArucoPoseTracker", "Read camera matrix: " << camMatrix.total() << " elements.");
  GM_DBG1("ArucoPoseTracker", "Read camera distortion: " << distCoeffs.total() << " elements.");

  return true;
}

void ArucoPoseTracker::traverse(Visitor *visitor) {
  for (auto &b:_impl->boards) b->accept(visitor);
  if (_impl->video_source) _impl->video_source->accept(visitor);
}

END_NAMESPACE_GMTRACK;

#endif
