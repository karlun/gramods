
#include <gmGraphics/PosedPlanarView.hh>
#include <gmCore/RunOnce.hh>
#include <gmCore/Console.hh>

#include <GL/glew.h>
#include <GL/gl.h>

BEGIN_NAMESPACE_GMGRAPHICS;

GM_OFI_DEFINE_SUB(PosedPlanarView, View);
GM_OFI_PARAM2(PosedPlanarView, fieldOfView, gmCore::angle2, setFieldOfView);

struct PosedPlanarView::Impl {

  void renderFullPipeline(ViewSettings settings);
  void renderFullPipeline(ViewSettings settings, Viewpoint *viewpoint);

  gmCore::angle2 field_of_view = { 1, -1 };
};

PosedPlanarView::PosedPlanarView() : _impl(std::make_unique<Impl>()) {}

void PosedPlanarView::setFieldOfView(gmCore::angle2 fov) {
  _impl->field_of_view = fov;
}

void PosedPlanarView::renderFullPipeline(ViewSettings settings) {
  populateViewSettings(settings);
  _impl->renderFullPipeline(settings);
}

void PosedPlanarView::Impl::renderFullPipeline(ViewSettings settings) {

  const float fov_h = field_of_view[0];
  const float fov_v = field_of_view[1];

  if (fov_h < 0 && fov_v < 0) {
    GM_RUNONCE(GM_ERR("PosedPlanarView", "Both horizontal and vertical field of view is set to be controlled by the other."));
    return;
  }

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  for (auto viewpoint : settings.viewpoints)
    renderFullPipeline(settings, viewpoint.get());
}

void PosedPlanarView::Impl::renderFullPipeline(ViewSettings settings,
                                               Viewpoint *viewpoint) {

  const float fov_h = field_of_view[0];
  const float fov_v = field_of_view[1];

  Eigen::Vector3f x_VP = viewpoint->getPosition();
  Eigen::Quaternionf q_VP = viewpoint->getOrientation();

  GLint cvp[4] = { 0, 0, 0, 0 };
  glGetIntegerv(GL_VIEWPORT, cvp);
  if (cvp[2] == 0 || cvp[3] == 0) {
    GM_ERR("PosedPlanarView", "Cannot render to degenerate viewport");
    return;
  }

  Camera camera(settings);

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

  settings.renderNodes(camera);
}

END_NAMESPACE_GMGRAPHICS;
