
#ifndef GRAMODS_TRACK_OPENCVVIDEOCAPTURE
#define GRAMODS_TRACK_OPENCVVIDEOCAPTURE

#include <gmTrack/config.hh>

#ifdef gramods_ENABLE_aruco

#include <gmTrack/OpenCvVideoSource.hh>
#include <gmCore/OFactory.hh>
#include <gmCore/Updateable.hh>

BEGIN_NAMESPACE_GMTRACK;

/**
   The OpenCvVideoCapture uses OpenCVs VideoCapture to read off video
   image data.

   This class configures as an Updateable with a priority of
   20. Either Updateable::updateAll or update must be called at even
   intervals. This is done automatically by gm-load.
*/
class OpenCvVideoCapture
  : public OpenCvVideoSource,
    public gmCore::Updateable {

public:

  OpenCvVideoCapture();

  /**
     Sets the video file to read from.
  */
  void setVideoFile(std::string file);

  /**
     Sets the camera id to capture data from.
  */
  void setCameraId(int id);

  /**
     Updates the video capture to read off the next frame.
  */
  void update(gmCore::Updateable::clock::time_point);

  /**
     Retrieve the latest read image captured.
     @param[out] image The latest image in the video source.
     @returns True if the image was successfully read.
  */
  bool retrieve(cv::Mat &image);

  GM_OFI_DECLARE;

private:

  struct Impl;
  std::unique_ptr<Impl> _impl;
};

END_NAMESPACE_GMTRACK;

#endif
#endif
