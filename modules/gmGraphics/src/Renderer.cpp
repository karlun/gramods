
#include <gmGraphics/Renderer.hh>

#include <limits>

BEGIN_NAMESPACE_GMGRAPHICS;

void Renderer::getNearFar(Renderer::list renderers, Camera camera,
                          float &near, float &far) {

  float total_near = std::numeric_limits<float>::max();
  float total_far = -1;
  for (auto renderer : renderers) {
    float near, far;
    renderer->getNearFar(camera, near, far);
    if (far < 0)
      continue;

    total_near = std::min(total_near, near);
    total_far = std::max(total_far, far);
  }

  if (total_far <= std::numeric_limits<float>::epsilon())
    total_far = 1.0f; // 1 m far plane if nothing better

  if (total_near <= std::numeric_limits<float>::epsilon())
    total_near = 0.01f; // 1 cm near plane if nothing better

  near = total_near;
  far = total_far;
}

END_NAMESPACE_GMGRAPHICS;
