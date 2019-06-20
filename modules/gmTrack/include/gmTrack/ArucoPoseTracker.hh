
#ifndef GRAMODS_TRACK_ARUCOPOSETRACKER
#define GRAMODS_TRACK_ARUCOPOSETRACKER

#include <gmTrack/config.hh>

#ifdef gramods_ENABLE_aruco

#include <gmCore/OFactory.hh>
#include <gmCore/Updateable.hh>

#include <gmTrack/SinglePoseTracker.hh>

#include <gmTrack/ArucoBoard.hh>
#include <gmTrack/OpenCvVideoSource.hh>

BEGIN_NAMESPACE_GMTRACK;

/**
   Pose tracker that tracks pose using feducial markers.

   This class configures as an Updateable with a priority of
   10. Either Updateable::updateAll or update must be called at even
   intervals. This is done automatically by gm-load.
*/
class ArucoPoseTracker
  : public SinglePoseTracker,
    public gmCore::Updateable {

public:

  ArucoPoseTracker();

  /**
     Updates the animation.
  */
  void update(gmCore::Updateable::clock::time_point t);

  //void setCornerRefineMethod(std::string m);
  //void setCornerRefineMethod(cv::aruco::CornerRefineMethod m);
  //void setDetectorParameters(cv::aruco::DetectorParameters params);
  //void setDetectorParametersFile(std::string file);
  //void setCameraParameters(cv::InputArray cameraMatrix cameraMatrix, cv::InputArray distCoeffs);
  //void setCameraParametersFile(std::string file);

  /**
     Set the ArucoBoard to track.

     \b XML-key: \c arucoBoard
  */
  void setArucoBoard(std::shared_ptr<ArucoBoard> board);

  /**
     Set the video source to read images from.

     \b XML-key: \c videoSource
  */
  void setVideoSource(std::shared_ptr<OpenCvVideoSource> vs);

  /**
     If set to true... Default is false.

     \b XML-attribute: \c inverted
  */
  void setInverted(bool on);

  /**
     Set to true to activate debug graphics.

     When true an OpenCV window will open showing marker detection and
     coordinate axes for the detected board.

     \b XML-attribute: \c showDebug
  */
  void setShowDebug(bool on);

  /**
     Set to true if markers that were not found should be detected
     based on the information from found markers. Default: false.

     \b XML-attribute: \c refindMarkers
  */
  void setRefindMarkers(bool on);

  /**
     Sets the (cv::FileStorage) file to read camera parameters from.

     \b XML-attribute: \c cameraConfigurationFile
  */
  void setCameraConfigurationFile(std::string file);

  /**
     Replaces the contents of p with pose data.
  */
  bool getPose(PoseSample &p);

  GM_OFI_DECLARE;

private:

  struct Impl;
  std::unique_ptr<Impl> _impl;
};

END_NAMESPACE_GMTRACK;

#endif

#endif
