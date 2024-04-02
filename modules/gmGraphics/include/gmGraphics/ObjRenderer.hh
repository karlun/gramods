
#ifndef GRAMODS_GRAPHICS_OBJRENDERER
#define GRAMODS_GRAPHICS_OBJRENDERER

#include <gmGraphics/Renderer.hh>

#include <gmCore/eigen.hh>
#include <gmCore/OFactory.hh>

BEGIN_NAMESPACE_GMGRAPHICS;

/**
   A renderer that draws an obj file.
*/
class ObjRenderer
  : public Renderer {

public:

  ObjRenderer();

  /**
     Performs rendering of 3D objects in the scene.
  */
  void render(Camera camera, float near = -1, float far = -1) override;

  /**
     Extracts the currently optimal near and far plane distances. This
     is typically used by a View if there are multiple renderers that
     need to be rendered with the same near and far planes for correct
     depth testing.
  */
  void getNearFar(Camera camera, float &near, float &far) override;

  /**
     Set the obj file to render.

     \gmXmlTag{gmGraphics,ObjRenderer,file}
  */
  void setFile(std::filesystem::path file);

  /**
     Set to true if the model should be recentered after
     loading. Default is false.

     \gmXmlTag{gmGraphics,ObjRenderer,recenter}
  */
  void setRecenter(bool on);

  /**
     Set the position of the object.

     \gmXmlTag{gmGraphics,ObjRenderer,position}

     \sa gramods::operator>>(std::istream &, Eigen::Vector3f &)
     \sa setTransform
  */
  void setPosition(Eigen::Vector3f p);

  /**
     Set the orientation of the object.

     \gmXmlTag{gmGraphics,ObjRenderer,orientation}

     \sa gramods::operator>>(std::istream &, Eigen::Quaternionf &)
     \sa setTransform
  */
  void setOrientation(Eigen::Quaternionf q);

  /**
     Set the model matrix of the rendered object. The functions
     setPosition and setOrientation can also be used to control the
     transform.

     \gmXmlTag{gmGraphics,ObjRenderer,transform}

     \sa gramods::operator>>(std::istream &, Eigen::Matrix4f &)
     \sa setPosition
     \sa setOrientation
  */
  void setTransform(Eigen::Matrix4f m);

  GM_OFI_DECLARE;

private:

  struct Impl;
  std::unique_ptr<Impl> _impl;

};

END_NAMESPACE_GMGRAPHICS;

#endif
