
#include <gmGraphics/SdlWindow.hh>

BEGIN_NAMESPACE_GMGRAPHICS;

GM_OFI_DEFINE(Window);
GM_OFI_PARAM(Window, fullscreen, bool, Window::setFullscreen);
GM_OFI_PARAM(Window, title, std::string, Window::setTitle);
GM_OFI_PARAM(Window, size, gmTypes::size2, Window::setSize);
GM_OFI_POINTER(Window, renderer, gmGraphics::Renderer, Window::addRenderer);

Window::Window()
  : fullscreen(false),
    title("untitled gramods window"),
    size(gmTypes::size2({1920, 1080})) {}

void Window::renderFullPipeline() {

  makeGLContextCurrent();

  if (!renderers_to_setup.empty())
    GM_VINF("Window", "setting up " << renderers_to_setup.size() << " renderers");
  while (!renderers_to_setup.empty()) {
    auto &renderer = renderers_to_setup.back();
    renderer->setup();
    renderers.push_back(renderer);
    renderers_to_setup.pop_back();
  }

  Camera c;
  GM_VINF("Window", "rendering " << renderers.size() << " renderers");
  for (auto renderer : renderers)
    renderer->render(c);
}

END_NAMESPACE_GMGRAPHICS;
