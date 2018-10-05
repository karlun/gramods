
#ifndef GRAMODS_GRAPHICS_FOVPLANARVIEW
#define GRAMODS_GRAPHICS_FOVPLANARVIEW

#include <gmGraphics/View.hh>

BEGIN_NAMESPACE_GMGRAPHICS;

/**
   This view provides skewed frustum projection of the renderers.
*/
class FovPlanarView
  : public View {

public:

  void renderFullPipeline(ViewSettings settings);

  /**
     Sets the clip planes, left, right, bottom, top, in x, y
     coordinates at distance 1.
  */
  void setClipPlanes(gmTypes::float4 p) {
    planes = p;
  }

  /**
     Angles between forward direction and the clip planes left, right,
     bottom and top, in radians. Positive angles are right and up.
  */
  void setClipAngles(gmTypes::float4 p) {
    planes[0] = tan(p[0]);
    planes[1] = tan(p[1]);
    planes[2] = tan(p[2]);
    planes[3] = tan(p[3]);
  }

  /**
     Sets the orientation of the View from Euler angles, roll, pitch,
     yaw, in radians.
  */
  void setEulerAngles(gmTypes::float3 e) {
    orientation =
      Eigen::AngleAxisf(e[0], Eigen::Vector3f::UnitX()) *
      Eigen::AngleAxisf(e[1], Eigen::Vector3f::UnitY()) *
      Eigen::AngleAxisf(e[2], Eigen::Vector3f::UnitZ());
  }

  /**
     Sets the orientation of the View as a quaternion in format (w x y
     z).
   */
  void setQuaternion(gmTypes::float4 rot) {
    orientation = Eigen::Quaternionf(rot[0], rot[1], rot[2], rot[3]);
  }

  GM_OFI_DECLARE;

private:

  gmTypes::float4 planes = { -1, 1, -1, 1 };
  Eigen::Quaternionf orientation = Eigen::Quaternionf::Identity();

};

END_NAMESPACE_GMGRAPHICS;

#endif
