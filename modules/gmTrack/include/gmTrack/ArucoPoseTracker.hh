
#ifndef GRAMODS_TRACK_ARUCOPOSETRACKER
#define GRAMODS_TRACK_ARUCOPOSETRACKER

#include <gmTrack/config.hh>

#ifdef gramods_ENABLE_OpenCV_objdetect

#include <gmCore/OFactory.hh>

#include <gmTrack/TrackerBase.hh>

#include <gmTrack/ArucoBoard.hh>
#include <gmTrack/OpenCvVideoCapture.hh>

BEGIN_NAMESPACE_GMTRACK;

/**
   Pose tracker that tracks pose using fiducial markers.

   This class configures as an Updateable with a priority of
   10. Either Updateable::updateAll or update must be called at even
   intervals. This is done automatically by gm-load.
*/
class ArucoPoseTracker : public PoseTracker {

public:

  ArucoPoseTracker();

  //void setCornerRefineMethod(std::string m);
  //void setCornerRefineMethod(cv::aruco::CornerRefineMethod m);
  //void setDetectorParameters(cv::aruco::DetectorParameters params);
  //void setDetectorParametersFile(std::string file);
  //void setCameraParameters(cv::InputArray cameraMatrix cameraMatrix, cv::InputArray distCoeffs);
  //void setCameraParametersFile(std::string file);

  /**
     Add an ArucoBoard to track.

     \gmXmlTag{gmTrack,ArucoPoseTracker,arucoBoard}
  */
  void addArucoBoard(std::shared_ptr<ArucoBoard> board);

  /**
     Set the video source to read images from.

     \gmXmlTag{gmTrack,ArucoPoseTracker,videoSource}
  */
  void setVideoSource(std::shared_ptr<OpenCvVideoCapture> vs);

  /**
     If set to true the pose tracker will estimate the camera pose
     relative the markers. When false the markers' poses are
     estimated. Default is false.

     \gmXmlTag{gmTrack,ArucoPoseTracker,trackCamera}
  */
  void setTrackCamera(bool on);

  /**
     Set the key to use for the data produced by this tracker. Board
     index will be appended. Default is "/aruco/T" where T is the
     current trackers' index.

     \gmXmlTag{gmTrack,ArucoPoseTracker,key}
  */
  void setKey(std::string key);

  /**
     Set to true to activate debug graphics.

     When true an OpenCV window will open showing marker detection and
     coordinate axes for the detected board.

     \gmXmlTag{gmTrack,ArucoPoseTracker,showDebug}
  */
  void setShowDebug(bool on);

  /**
     Set to true if markers that were not found should be detected
     based on the information from found markers. Default: false.

     \gmXmlTag{gmTrack,ArucoPoseTracker,refindMarkers}
  */
  void setRefindMarkers(bool on);

  /**
     Sets the (cv::FileStorage) file to read camera parameters from.

     \gmXmlTag{gmTrack,ArucoPoseTracker,cameraConfigurationFile}
  */
  void setCameraConfigurationFile(std::filesystem::path file);

  /**
     @see TrackerBase::get
  */
  std::optional<State> get() override;

  /**
     Propagates the specified visitor.

     @see Object::Visitor
  */
  void traverse(Visitor *visitor) override;

  GM_OFI_DECLARE;

private:

  struct Impl;
  std::unique_ptr<Impl> _impl;
};

END_NAMESPACE_GMTRACK;

#endif

#endif
