
#ifndef GRAMODS_GRAPHICS_POSETRANSFORM
#define GRAMODS_GRAPHICS_POSETRANSFORM

// Required before gmCore/OFactory.hh for some compilers
#include <gmCore/io_eigen.hh>

#include <gmGraphics/Transform.hh>

BEGIN_NAMESPACE_GMGRAPHICS;

/**
   The scenegraph PoseTransform base.
*/
class PoseTransform : public Transform {

public:
  PoseTransform();
  ~PoseTransform();

  /**
     Set the position of the childrens' space.

     \gmXmlTag{gmGraphics,PoseTransform,position}

     \sa gramods::operator>>(std::istream &, Eigen::Vector3f &)
  */
  void setPosition(Eigen::Vector3f p);

  /**
     Set the orientation of the childrens' space.

     \gmXmlTag{gmGraphics,PoseTransform,orientation}

     \sa gramods::operator>>(std::istream &, Eigen::Quaternionf &)
  */
  void setOrientation(Eigen::Quaternionf q);

  /**
     Set the center of the orientation change.

     \gmXmlTag{gmGraphics,PoseTransform,orientationCenter}

     \sa gramods::operator>>(std::istream &, Eigen::Vector3f &)
  */
  void setOrientationCenter(Eigen::Vector3f c);

  /**
     Set the scale of the childrens' space.

     \gmXmlTag{gmGraphics,PoseTransform,scale}

     \sa gramods::operator>>(std::istream &, Eigen::Vector3f &)
  */
  void setScale(Eigen::Vector3f p);

  /**
     Set the center of the scale change.

     \gmXmlTag{gmGraphics,PoseTransform,scaleCenter}

     \sa gramods::operator>>(std::istream &, Eigen::Vector3f &)
  */
  void setScaleCenter(Eigen::Vector3f c);

  /**
     Set the orientation of the scale vector, in local space.

     \gmXmlTag{gmGraphics,PoseTransform,scaleOrientation}

     \sa gramods::operator>>(std::istream &, Eigen::Vector3f &)
  */
  void setScaleOrientation(Eigen::Quaternionf q);

  /**
     \sa Transform::getTransform
  */
  Eigen::Affine3f getTransform() override;

  GM_OFI_DECLARE;

private:
  struct Impl;
  std::unique_ptr<Impl> _impl;
};

END_NAMESPACE_GMGRAPHICS;

#endif
