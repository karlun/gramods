
#ifndef GRAMODS_TRACK_ARUCOBOARDCOMPLEX
#define GRAMODS_TRACK_ARUCOBOARDCOMPLEX

#include <gmTrack/ArucoBoard.hh>

#ifdef gramods_ENABLE_aruco

#include <gmTypes/eigen.hh>
#include <gmCore/OFactory.hh>

#include <Eigen/Eigen>

BEGIN_NAMESPACE_GMTRACK;

/**
   The Aruco board complex creates a potentially non-flat Aruco
   "board", or rig, by combinging a set of Aruco boards with different
   pose relative to the complex base.

   Either only boards and no pose specifications are provided, and the
   specified boards have different positions and/or orientations, or
   an equal amount of boards and pose specifications must be provided.
*/
class ArucoBoardComplex : public ArucoBoard {

public:

  ArucoBoardComplex();

  /**
     Add an Aruco board to the complex.

     Either only boards and no pose specifications are provided, and
     the specified boards have different positions and/or
     orientations, or an equal amount of boards and pose
     specifications must be provided.
  */
  void addArucoBoard(std::shared_ptr<ArucoBoard> b);

  /**
     Add a board position of the complex, in xml as (x y z).

     Either only boards and no pose specifications are provided, and
     the specified boards have different positions and/or
     orientations, or an equal amount of boards and pose
     specifications must be provided.
  */
  void addPosition(Eigen::Vector3f p);

  /**
     Add a board orientation, as quaternion, in xml as (w x y z).

     Either only boards and no pose specifications are provided, and
     the specified boards have different positions and/or
     orientations, or an equal amount of boards and pose
     specifications must be provided.
  */
  void addQuaternion(Eigen::Quaternionf q);

  /**
     Add a board orientation, as an angle axis rotation, in xml as (a
     x y z), where angle a is expressed in radians.

     Either only boards and no pose specifications are provided, and
     the specified boards have different positions and/or
     orientations, or an equal amount of boards and pose
     specifications must be provided.
  */
  void addAngleAxis(Eigen::AngleAxisf q);

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
