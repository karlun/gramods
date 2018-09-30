
#include <gmGraphics/CubeSceneRenderer.hh>

BEGIN_NAMESPACE_GMGRAPHICS;

GM_OFI_DEFINE(CubeSceneRenderer);
GM_OFI_PARAM(CubeSceneRenderer, cubeSize, float, CubeSceneRenderer::setCubeSize);
GM_OFI_PARAM(CubeSceneRenderer, cubeSetSize, float, CubeSceneRenderer::setCubeSetSize);
GM_OFI_PARAM(CubeSceneRenderer, cubeSetCenter, gmTypes::float3, CubeSceneRenderer::setCubeSetCenter);

struct CubeSceneRenderer::Impl {

  ~Impl();

  void setup();
  void render(Camera camera);

  float cube_size = 0.1;
  float cube_set_size = 1.0;
  gmTypes::float3 cube_set_center = { 0, 0, 0 };
  bool has_been_setup = false;
};

CubeSceneRenderer::CubeSceneRenderer()
  : _impl(std::make_unique<Impl>()) {}

void CubeSceneRenderer::render(Camera camera) {
  _impl->render(camera);
}

void CubeSceneRenderer::Impl::setup() {

  size_t N = (size_t)(cube_set_size / (3 * cube_size));

  // Create GL data here

  has_been_setup = true;
}

void CubeSceneRenderer::Impl::render(Camera camera) {
  if (!has_been_setup)
    setup();

  // Render GL data here
}

void CubeSceneRenderer::setCubeSize(float d) {
  _impl->cube_size = d;
  _impl->has_been_setup = false;
}

void CubeSceneRenderer::setCubeSetSize(float d) {
  _impl->cube_set_radius = d;
  _impl->has_been_setup = false;
}

void CubeSceneRenderer::setCubeSetCenter(gmTypes::float3 c) {
  _impl->cube_set_center = c;
  _impl->has_been_setup = false;
}

END_NAMESPACE_GMGRAPHICS;
