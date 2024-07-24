
#ifndef GRAMODS_GRAPHICS_MATRIXTRANSFORM
#define GRAMODS_GRAPHICS_MATRIXTRANSFORM

// Required before gmCore/OFactory.hh for some compilers
#include <gmCore/io_eigen.hh>

#include <gmGraphics/Transform.hh>

BEGIN_NAMESPACE_GMGRAPHICS;

/**
   The scenegraph MatrixTransform base.
*/
class MatrixTransform : public Transform {

public:
  MatrixTransform();
  ~MatrixTransform();

  /**
     Set the matrix of the childrens' space.

     \gmXmlTag{gmGraphics,MatrixTransform,matrix}

     \sa gramods::operator>>(std::istream &, Eigen::Matrix4f &)
  */
  void setMatrix(Eigen::Matrix4f m);

  /**
     Returns the matrix of the childrens' space.
  */
  Eigen::Matrix4f getMatrix() const;

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
