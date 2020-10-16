
#include <gmGraphics/Renderer.hh>

#include <limits>

BEGIN_NAMESPACE_GMGRAPHICS;

void Renderer::getNearFar(Renderer::list renderers, Camera camera,
                          float &near, float &far) {

  float total_near = std::numeric_limits<float>::max();
  float total_far = -1;
  for (auto renderer : renderers) {
    float near = -1, far = -1;
    renderer->getNearFar(camera, near, far);

    if (near > 0) total_near = std::min(total_near, near);
    if (far > 0) total_far = std::max(total_far, far);
  }

  if (total_near = std::numeric_limits<float>::max() ||
      total_near <= std::numeric_limits<float>::epsilon())
    // 1 cm near plane if nothing better
    total_near = 0.01f;

  if (total_far < total_near + std::numeric_limits<float>::epsilon())
    // far plane 1 m behind near if nothing better
    total_far = total_near + 1.0f;

  near = total_near;
  far = total_far;
}

END_NAMESPACE_GMGRAPHICS;
