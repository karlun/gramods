
#ifndef GRAMODS_GRAPHICS_PROCESSOR
#define GRAMODS_GRAPHICS_PROCESSOR

#include <gmGraphics/config.hh>

#include <gmCore/Object.hh>
#include <gmGraphics/Camera.hh>

#include <globjects/Texture.hh>

BEGIN_NAMESPACE_GMGRAPHICS;

/**
   The base of processor implementations, classes with the role of
   taking graphics projections (in the form of texture objects) from
   specified cameras.
*/
class Processor
  : public gmCore::Object {

  /**
     Adds a camera specifying a view that should be rendered by this
     processor.
  */
  void addOutputCamera(Camera camera);

  /**
     Sets the cameras that specify the views that should be rendered
     by this processor.
  */
  bool setOutputCameras(std::vector<Camera> cameras);

  /**
     Gets the cameras specifying the views required by this processor
     to produce its output views.
  */
  bool getInputCameras(std::vector<Camera> cameras);

  /**
     Renders the views specified by the available output cameras.

     The default implementation calls render(Camera,
     globjects::Texture) once for each available output camera and
     returns true. It returns false only if no output camera is
     available. A sub class may replace this render method to perform
     the rendering in other ways and return true or false on other
     premises.

     \param[out] projs The textures containing the views representing
     the specified output cameras.

     \return True if successful, false otherwise.
  */
  virtual bool render(std::vector<globjects::Texture> &projs);

  /**
     Renders the views represented by the specified camera.

     \param[in] cam The camera to render.

     \param[out] proj The resulting view, representing the specified
     output camera.
   */
  virtual void render(Camera cam, globjects::Texture &proj) = 0;

protected:

  /**
     The cameras that this processor should try to render views for
     upon calls to the render method.
  */
  std::vector<Camera> output_cameras;

};

END_NAMESPACE_GMGRAPHICS;

#endif
