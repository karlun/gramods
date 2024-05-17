
#ifndef GRAMODS_GRAPHICS_OBJRENDERER
#define GRAMODS_GRAPHICS_OBJRENDERER

#include <gmGraphics/Renderer.hh>

#include <gmCore/io_eigen.hh>
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
     Called to initialize the Object. This should be called once only!
  */
  void initialize() override;

  /**
     Performs rendering of 3D objects in the scene.
  */
  void render(const Camera &camera, const Eigen::Affine3f &Mm) override;

  /**
     Extracts the currently optimal near and far plane distances. This
     is typically used by a View if there are multiple renderers that
     need to be rendered with the same near and far planes for correct
     depth testing.
  */
  void getNearFar(const Camera &camera,
                  const Eigen::Affine3f &Mm,
                  float &near,
                  float &far) override;

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

  GM_OFI_DECLARE;

private:

  struct Impl;
  std::unique_ptr<Impl> _impl;

};

END_NAMESPACE_GMGRAPHICS;

#endif
