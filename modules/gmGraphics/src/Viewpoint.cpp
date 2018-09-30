
#include <gmGraphics/Viewpoint.hh>

BEGIN_NAMESPACE_GMGRAPHICS;

GM_OFI_DEFINE(Viewpoint);
GM_OFI_PARAM(Viewpoint, position, gmTypes::float3, Viewpoint::setPosition);
GM_OFI_PARAM(Viewpoint, quaternion, gmTypes::float4, Viewpoint::setQuaternion);
GM_OFI_PARAM(Viewpoint, axisAngle, gmTypes::float4, Viewpoint::setAxisAngle);

END_NAMESPACE_GMGRAPHICS;
