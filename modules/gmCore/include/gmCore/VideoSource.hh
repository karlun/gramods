
#ifndef GRAMODS_CORE_VIDEOSOURCE
#define GRAMODS_CORE_VIDEOSOURCE

#include <gmCore/config.hh>

#ifdef gramods_ENABLE_OpenCV

#include <gmCore/size.hh>

#include <opencv2/opencv.hpp>

BEGIN_NAMESPACE_GMCORE;

/**
   The VideoSource is an interface image data as OpenCV and/or Eigen
   matrix.
*/
class VideoSource {

public:

  /**
     Trigger a still image capture. Returns false iff still image
     capture could not be triggered.
  */
  virtual bool triggerStill(gmCore::size2 res) = 0;

  /**
     Retrieve the latest read image in the video source in OpenCV
     format.

     @param[out] image The latest image in the video source.

     @returns True if the image was successfully read.
  */
  virtual bool retrieve(cv::Mat &image) = 0;

};

END_NAMESPACE_GMCORE;

#endif
#endif
