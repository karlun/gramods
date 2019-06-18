
#ifndef GRAMODS_TRACK_ARUCOGRIDBOARD
#define GRAMODS_TRACK_ARUCOGRIDBOARD

#include <gmTrack/ArucoBoard.hh>

#ifdef gramods_ENABLE_aruco

#include <gmTypes/eigen.hh>
#include <gmTypes/float.hh>
#include <gmCore/OFactory.hh>

#include <Eigen/Eigen>

BEGIN_NAMESPACE_GMTRACK;

/**
   The Aruco grid board creates a flat grid of fiducial markers.

   The coordinate system is centered for convenience when used
   together with ArucoBoardComplex to form complex structures.
*/
class ArucoGridBoard : public ArucoBoard {

public:

  ArucoGridBoard();

  /**
     Set the number of columns of markers to track. Default is 1.
  */
  void setColumns(size_t N);

  /**
     Set the number of columns of markers to track. Default is 1.
  */
  void setRows(size_t N);

  /**
     Set the size of the markers, typically in meters.
  */
  void setMarkerSize(float s);

  /**
     Set the distance between the markers, typically in
     meters.
  */
  void setMarkerSeparation(float s);

  /**
     Set the first id to use from the dictionary when defining the markers.

     Since the ids are taken in sequence from the dictionary, the last
     id will be id + columns x rows - 1. Default is 0.
  */
  void setFirstId(size_t id);

  /**
     Set dictionary to pull marker definition from.

     Default is "ARUCO_ORIGINAL" and valid values are

     - 4X4_50
     - 4X4_100
     - 4X4_250
     - 4X4_1000
     - 5X5_50
     - 5X5_100
     - 5X5_250
     - 5X5_1000
     - 6X6_50
     - 6X6_100
     - 6X6_250
     - 6X6_1000
     - 7X7_50
     - 7X7_100
     - 7X7_250
     - 7X7_1000
     - ARUCO_ORIGINAL
     - APRILTAG_16h5
     - APRILTAG_25h9
     - APRILTAG_36h10
     - APRILTAG_36h11
  */
  void setDictionary(std::string dict);

  /**
     Set the position of the board grid relative to the origin of the
     board.
  */
  void setPosition(Eigen::Vector3f p);

  /**
     Set the orientation of the board grid as quaternion, in xml as (w
     x y z).
  */
  void setQuaternion(Eigen::Quaternionf q);

  /**
     Set the orientation of the board grid as an angle axis, in xml as
     (a x y z) where angle a is expressed in radians.
  */
  void setAngleAxis(Eigen::AngleAxisf aa);

  /**
     Set the orientation of the board grid, as euler angles.
  */
  void setEulerAngles(gmTypes::float3 e);

  /**
     Returns a reference to the board defined by this node.
  */
  cv::Ptr<cv::aruco::Board> getBoard();

  GM_OFI_DECLARE;

private:

  struct Impl;
  std::unique_ptr<Impl> _impl;
};

END_NAMESPACE_GMTRACK;

#endif
#endif
