
#ifndef GRAMODS_TRACK_OPENCVVIDEOSOURCE
#define GRAMODS_TRACK_OPENCVVIDEOSOURCE

#include <gmTrack/config.hh>

#ifdef gramods_ENABLE_aruco

#include <gmCore/Object.hh>
#include <gmCore/OFactory.hh>

#include <opencv2/opencv.hpp>

BEGIN_NAMESPACE_GMTRACK;

/**
   The OpenCvVideoSource is the base of nodes providing OpenCV image
   data.
*/
class OpenCvVideoSource
  : public gmCore::Object {

public:

  /**
     Retrieve the latest read image in the video source.
     @param[out] image The latest image in the video source.
     @returns True if the image was successfully read.
  */
  virtual bool retrieve(cv::Mat &image) = 0;

  /**
     Returns the default key, in Configuration, for the
     Object.
  */
  std::string getDefaultKey() { return "videoSource"; }

};

END_NAMESPACE_GMTRACK;

#endif
#endif
