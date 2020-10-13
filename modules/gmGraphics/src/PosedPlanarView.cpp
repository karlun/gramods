
#include <gmGraphics/PosedPlanarView.hh>
#include <gmCore/RunOnce.hh>

#include <GL/glew.h>
#include <GL/gl.h>

BEGIN_NAMESPACE_GMGRAPHICS;

GM_OFI_DEFINE_SUB(PosedPlanarView, View);
GM_OFI_PARAM(PosedPlanarView, fieldOfView, gmCore::angle2, PosedPlanarView::setFieldOfView);

void PosedPlanarView::renderFullPipeline(ViewSettings settings) {

  float fov_h = field_of_view[0];
  float fov_v = field_of_view[1];

  if (fov_h < 0 && fov_v < 0) {
    GM_RUNONCE(GM_ERR("PosedPlanarView", "Both horizontal and vertical field of view is set to be controlled by the other."));
    return;
  }

  populateViewSettings(settings);

  Eigen::Vector3f x_VP = Eigen::Vector3f::Zero();
  Eigen::Quaternionf q_VP = Eigen::Quaternionf::Identity();

  if (settings.viewpoint) {
    x_VP = settings.viewpoint->getPosition();
    q_VP = settings.viewpoint->getOrientation();
  } else {
    GM_RUNONCE(GM_WRN("PosedPlanarView", "No viewpoint available - using zero position and rotation"));
  }

  GLint cvp[4] = { 0, 0, 0, 0 };
  glGetIntegerv(GL_VIEWPORT, cvp);
  if (cvp[2] == 0 || cvp[3] == 0) {
    GM_ERR("PosedPlanarView", "Cannot render to degenerate viewport");
    return;
  }

  Camera camera;

  if (fov_h < 0) {

    float top = tanf(0.5f * fov_v);

    float ratio = cvp[2] / (float)cvp[3];
    float right = ratio * top;

    camera.setClipPlanes(-right, right, -top, top);

  } else if (fov_v < 0) {

    float right = tanf(0.5f * fov_h);

    float ratio = cvp[3] / (float)cvp[2];
    float top = ratio * right;

    camera.setClipPlanes(-right, right, -top, top);

  } else {
    camera.setFieldOfView(fov_h, fov_v);
  }

  camera.setPose(x_VP, q_VP);

  float near, far;
  Renderer::getNearFar(settings.renderers, camera, near, far);

  for (auto renderer : settings.renderers)
    renderer->render(camera, near, far);
}

END_NAMESPACE_GMGRAPHICS;
