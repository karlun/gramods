
#ifndef GRAMODS_TRACK_OPENCVVIDEOCAPTURE
#define GRAMODS_TRACK_OPENCVVIDEOCAPTURE

#include <gmTrack/config.hh>

#ifdef gramods_ENABLE_OpenCV

#include <gmCore/VideoSource.hh>

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
  : public gmCore::Object,
    public gmCore::VideoSource,
    public gmCore::Updateable {

public:

  OpenCvVideoCapture();

  /**
     Sets the video file to read from.

     \gmXmlTag{gmTrack,OpenCvVideoCapture,videoFile}
  */
  void setVideoFile(std::filesystem::path file);

  /**
     Sets the camera id to capture data from.

     \gmXmlTag{gmTrack,OpenCvVideoCapture,cameraId}
  */
  void setCameraId(int id);

  /**
     Sets the width of the resolution to request from the camera.

     If not set the driver default is used.

     \gmXmlTag{gmTrack,OpenCvVideoCapture,cameraWidth}
  */
  void setCameraWidth(int W);

  /**
     Sets the height of the resolution to request from the camera.

     If not set the driver default is used.

     \gmXmlTag{gmTrack,OpenCvVideoCapture,cameraHeight}
  */
  void setCameraHeight(int W);

  /**
     Sets the framerate to request from the camera.

     If not set the driver default is used.

     \gmXmlTag{gmTrack,OpenCvVideoCapture,cameraFramerate}

     @see https://www.fourcc.org/codecs.php
  */
  void setCameraFramerate(int R);

  /**
     Specifies the FourCC for the camera stream.

     If not set the driver default is used. A value supported by many
     cameras is "MJPG". This string must be four characters long, so
     add spaces when necessary.

     \gmXmlTag{gmTrack,OpenCvVideoCapture,cameraFourCC}
  */
  void setCameraFourCC(std::string cc);

  /**
     Specifies backend to use for reading camera or video stream.

     Default is "ANY" and the following are all valid alternatives,
     though not all may be supported by the current build of OpenCV:

     - ANY
     - V4L
     - V4L2
     - FIREWIRE
     - FIREWARE
     - IEEE1394
     - DC1394
     - CMU1394
     - DSHOW
     - PVAPI
     - OPENNI
     - OPENNI_ASUS
     - ANDROID
     - XIAPI
     - AVFOUNDATION
     - GIGANETIX
     - MSMF
     - WINRT
     - INTELPERC
     - REALSENSE
     - OPENNI2
     - OPENNI2_ASUS
     - GPHOTO2
     - GSTREAMER
     - FFMPEG
     - IMAGES
     - ARAVIS
     - OPENCV_MJPEG
     - INTEL_MFX
     - XINE

     \gmXmlTag{gmTrack,OpenCvVideoCapture,backend}
  */
  void setBackend(std::string b);

  /**
     Updates the video capture to read off the next frame.
  */
  void update(gmCore::Updateable::clock::time_point, size_t);

  /**
     Retrieve the latest read image captured.

     @param[out] image The latest image in the video source.

     @returns True if the image was successfully read.
  */
  bool retrieve(cv::Mat &image) override;

  /**
     Returns the default key, in Configuration, for the Object.
  */
  std::string getDefaultKey() override { return "videoSource"; }

  GM_OFI_DECLARE;

private:

  struct Impl;
  std::unique_ptr<Impl> _impl;
};

END_NAMESPACE_GMTRACK;

#endif
#endif
