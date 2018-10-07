
#include <gmGraphics/FovPlanarView.hh>

BEGIN_NAMESPACE_GMGRAPHICS;

GM_OFI_DEFINE_SUB(FovPlanarView, View);
GM_OFI_PARAM(FovPlanarView, clipPlanes, gmTypes::float4, FovPlanarView::setClipPlanes);
GM_OFI_PARAM(FovPlanarView, clipAngles, gmTypes::float4, FovPlanarView::setClipAngles);
GM_OFI_PARAM(FovPlanarView, eulerAngles, gmTypes::float3, FovPlanarView::setEulerAngles);
GM_OFI_PARAM(FovPlanarView, quaternion, gmTypes::float4, FovPlanarView::setQuaternion);


void FovPlanarView::renderFullPipeline(ViewSettings settings) {

  if (viewpoint)
    settings.viewpoint = viewpoint;
  settings.renderers.insert(settings.renderers.end(),
                            renderers.begin(), renderers.end());

  Eigen::Vector3f x_VP = Eigen::Vector3f::Zero();
  Eigen::Quaternionf q_VP = Eigen::Quaternionf::Identity();

  if (settings.viewpoint) {
    x_VP = settings.viewpoint->getPosition();
    q_VP = settings.viewpoint->getOrientation();
  }

  Camera camera;
  camera.setPlanes(planes[0], planes[1], planes[2], planes[3]);
  camera.setPose(x_VP, orientation);

  for (auto renderer : settings.renderers)
    renderer->render(camera);
}

END_NAMESPACE_GMGRAPHICS;
