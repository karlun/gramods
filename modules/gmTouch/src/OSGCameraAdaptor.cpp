
#include <gmTouch/OSGCameraAdaptor.hh>

#ifdef gramods_ENABLE_OpenSceneGraph

BEGIN_NAMESPACE_GMTOUCH;

bool OSGCameraAdaptor::setCurrentProjection(osg::Camera * camera) {

  osg::Matrix VPW = (camera->getViewMatrix()
                     * camera->getProjectionMatrix()
                     * camera->getViewport()->computeWindowMatrix());
  osg::Matrix invVPW;
  if (!invVPW.invert(VPW))
    return false;

  Eigen::Matrix4f WPV_inv = Eigen::Map<Eigen::Matrix4d>(invVPW.ptr()).cast<float>();
  CameraAdaptor::setCurrentProjection(WPV_inv);

  return true;
}

END_NAMESPACE_GMTOUCH;

#endif
