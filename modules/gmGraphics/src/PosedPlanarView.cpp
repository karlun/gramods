
#include <gmGraphics/PosedPlanarView.hh>

#include <GL/glew.h>
#include <GL/gl.h>

BEGIN_NAMESPACE_GMGRAPHICS;

GM_OFI_DEFINE_SUB(PosedPlanarView, View);
GM_OFI_PARAM(PosedPlanarView, fieldOfView, float, PosedPlanarView::setFieldOfView);

void PosedPlanarView::renderFullPipeline(ViewSettings settings) {

  if (viewpoint)
    settings.viewpoint = viewpoint;
  settings.renderers.insert(settings.renderers.end(),
                            renderers.begin(), renderers.end());

  Eigen::Vector3f x_VP = Eigen::Vector3f::Zero();
  Eigen::Quaternionf q_VP = Eigen::Quaternionf::Identity();

  if (settings.viewpoint) {
    x_VP = settings.viewpoint->getPosition();
    q_VP = settings.viewpoint->getOrientation();
  } else {
    static bool message_shown = false;
    if (!message_shown)
      GM_WRN("PosedPlanarView", "No viewpoint available - using zero position and rotation");
    message_shown = true;
  }

  GLint cvp[4] = { 0, 0, 0, 0 };
  glGetIntegerv(GL_VIEWPORT, cvp);
  if (cvp[2] == 0 || cvp[3] == 0) {
    GM_ERR("PosedPlanarView", "Cannot render to degenerate viewport");
    return;
  }

  float ratio = cvp[2] / cvp[3];

  Camera camera;
  camera.setFieldOfView(ratio * field_of_view, field_of_view);
  camera.setPose(x_VP, q_VP);

  for (auto renderer : settings.renderers)
    renderer->render(camera);
}

END_NAMESPACE_GMGRAPHICS;
