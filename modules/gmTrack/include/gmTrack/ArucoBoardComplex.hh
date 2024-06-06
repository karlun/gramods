
#ifndef GRAMODS_TRACK_ARUCOBOARDCOMPLEX
#define GRAMODS_TRACK_ARUCOBOARDCOMPLEX

#include <gmTrack/ArucoBoard.hh>

#ifdef gramods_ENABLE_OpenCV_objdetect

#include <gmCore/io_eigen.hh>
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
     
     \gmXmlTag{gmTrack,ArucoBoardComplex,arucoBoard}
  */
  void addArucoBoard(std::shared_ptr<ArucoBoard> b);

  /**
     Add a board position of the complex, in xml as (x y z).

     Either only boards and no pose specifications are provided, and
     the specified boards have different positions and/or
     orientations, or an equal amount of boards and pose
     specifications must be provided. The offset follows OpenGL
     convention: x left, y up and z back (out of the board).
     
     \gmXmlTag{gmTrack,ArucoBoardComplex,position}
  */
  void addPosition(Eigen::Vector3f p);

  /**
     Add a board orientation.

     Either only boards and no pose specifications are provided, and
     the specified boards have different positions and/or
     orientations, or an equal amount of boards and pose
     specifications must be provided.

     \gmXmlTag{gmTrack,ArucoBoardComplex,orientation}

     \sa gramods::operator>>(std::istream &, Eigen::Quaternionf &)
  */
  void addOrientation(Eigen::Quaternionf q);

  /**
     Returns a reference to the board defined by this node.
  */
  cv::Ptr<cv::aruco::Board> getBoard() override;

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
