
#include <gmGraphics/FovPlanarView.hh>

BEGIN_NAMESPACE_GMGRAPHICS;

GM_OFI_DEFINE_SUB(FovPlanarView, View);
GM_OFI_PARAM2(FovPlanarView, clipPlanes, gmCore::float4, setClipPlanes);
GM_OFI_PARAM2(FovPlanarView, clipAngles, gmCore::angle4, setClipAngles);
GM_OFI_PARAM2(FovPlanarView, orientation, Eigen::Quaternionf, setOrientation);


void FovPlanarView::renderFullPipeline(ViewSettings settings) {

  populateViewSettings(settings);

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  for (auto viewpoint : settings.viewpoints) {
    Eigen::Vector3f x_VP = viewpoint->getPosition();
    Eigen::Quaternionf q_VP = viewpoint->getOrientation();

    Camera camera(settings);
    camera.setClipPlanes(planes[0], planes[1], planes[2], planes[3]);
    camera.setPose(x_VP, orientation);

    settings.renderNodes(camera);
  }
}

END_NAMESPACE_GMGRAPHICS;
