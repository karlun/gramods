
#ifndef GRAMODS_TOUCH_OSGCAMERAADAPTOR
#define GRAMODS_TOUCH_OSGCAMERAADAPTOR

#include <gmTouch/config.hh>

#ifdef gramods_ENABLE_OpenSceneGraph

#include <osg/Camera>

#include <gmTouch/TouchState.hh>

BEGIN_NAMESPACE_GMTOUCH;

/**
 * Camera adaptor for OSG, providing means to set the projection
 * matrix of TouchState by providing an OSG camera.
 *
 * Typical use:
 * \code{.cpp}
 * int width, height;
 * SDL_GetWindowSize(sdl_window, &width, &height);
 *
 * touchState.eventsInit(width, height);
 * touchState.getCameraAdaptor<gmTouch::OSGCameraAdaptor>().setCurrentProjection(camera);
 * ...
 * touchState.eventsDone();
 * \endcode
 */
class OSGCameraAdaptor
  : public TouchState::CameraAdaptor {

public:

  /**
   * Calculates and sets the current projection matrix to use when
   * back-projecting the 2D touch points into 3D, and set the
   * previously current to be used when back-projecting "previous"
   * touch points.
   *
   * If the view changes over time, one of these methods needs to be
   * called every frame, between calling eventsInit and eventsDone
   * and before extrating 3D lines. If the view is static, it needs
   * to be called at least twice, to make the current matrix be
   * copied to previous state. The frame copying is performed even
   * when the valid project cannot be extracted.
   *
   * @return True if a valid projection could be extracted from the
   * specified camera, false otherwise.
   *
   * \see TouchState::getCurrent3DTouchLine
   */
  bool setCurrentProjection(osg::Camera * camera);

};

END_NAMESPACE_GMTOUCH;

#endif
#endif
