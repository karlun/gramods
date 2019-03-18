
#ifndef GRAMODS_TRACK_ARUCOBOARDCOMPLEX
#define GRAMODS_TRACK_ARUCOBOARDCOMPLEX

#include <gmTrack/ArucoBoard.hh>

#ifdef gramods_ENABLE_aruco

#include <gmTypes/all.hh>
#include <gmCore/OFactory.hh>

#include <Eigen/Eigen>

BEGIN_NAMESPACE_GMTRACK;

/**
   The Aruco board complex creates a potentially non-flat Aruco board
   by combinging a set of Aruco boards with different pose relative to
   the complex base.
*/
class ArucoBoardComplex : public ArucoBoard {

public:

  ArucoBoardComplex();

  /**
     Add an Aruco board to the complex.

     An equal amount of boards and pose specifications must be
     provided.
  */
  void addArucoBoard(std::shared_ptr<ArucoBoard> b);

  /**
     Add a board position of the complex.

     An equal amount of boards and pose specifications must be
     provided.
  */
  void addPosition(gmTypes::float3 p);

  /**
     Add a board orientation, as quaternion in format (w x y z).

     An equal amount of boards and pose specifications must be
     provided.
  */
  void addQuaternion(gmTypes::float4 q);

  /**
     Add a board orientation, as an axis angle in format (x y z a)
     where angle a is expressed in radians.

     An equal amount of boards and pose specifications must be
     provided.
  */
  void addAxisAngle(gmTypes::float4 q);

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
