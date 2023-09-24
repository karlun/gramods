
#ifndef GRAMODS_TRACK_ARUCOBOARD
#define GRAMODS_TRACK_ARUCOBOARD

#include <gmTrack/config.hh>

#ifdef gramods_ENABLE_OpenCV_objdetect

#include <gmCore/Object.hh>

#include <opencv2/objdetect.hpp>

BEGIN_NAMESPACE_GMTRACK;

/**
   The ArucoBoard is used by the ArucoTracker node to get the board
   configuration to track.
*/
class ArucoBoard
  : public gmCore::Object {

public:

  /**
     Returns a reference to the board defined by this node.
  */
  virtual cv::Ptr<cv::aruco::Board> getBoard() = 0;

  /**
     Returns the default key, in Configuration, for the
     Object.
  */
  std::string getDefaultKey() override { return "arucoBoard"; }

};

END_NAMESPACE_GMTRACK;

#endif
#endif
