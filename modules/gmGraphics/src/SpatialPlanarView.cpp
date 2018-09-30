
#include <gmGraphics/SpatialPlanarView.hh>

BEGIN_NAMESPACE_GMGRAPHICS;

GM_OFI_DEFINE(SpatialPlanarView);
GM_OFI_PARAM(SpatialPlanarView, topLeftCorner, gmTypes::float3, SpatialPlanarView::setTopLeftCorner);
GM_OFI_PARAM(SpatialPlanarView, bottomRightCorner, gmTypes::float3, SpatialPlanarView::setBottomRightCorner);
GM_OFI_PARAM(SpatialPlanarView, upDirection, gmTypes::float3, SpatialPlanarView::setUpDirection);

void SpatialPlanarView::renderFullPipeline(ViewSettings settings) {
  
}

END_NAMESPACE_GMGRAPHICS;
