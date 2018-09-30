
#include <gmGraphics/Window.hh>
#include <gmGraphics/View.hh>

BEGIN_NAMESPACE_GMGRAPHICS;

GM_OFI_DEFINE(Window);
GM_OFI_PARAM(Window, fullscreen, bool, Window::setFullscreen);
GM_OFI_PARAM(Window, title, std::string, Window::setTitle);
GM_OFI_PARAM(Window, size, gmTypes::size2, Window::setSize);
GM_OFI_POINTER(Window, view, gmGraphics::View, Window::addView);

Window::Window()
  : fullscreen(false),
    title("untitled gramods window"),
    size(gmTypes::size2({640, 480})) {}

void Window::renderFullPipeline(ViewSettings settings) {
  makeGLContextCurrent();
  RendererDispatcher::renderFullPipeline(settings);

  GM_VINF("Window", "Got " << settings.renderers.size() << " and adding " << renderers.size() << " renderers");
  settings.renderers.insert(settings.renderers.end(),
                            renderers.begin(), renderers.end());

  if (viewpoint)
    settings.viewpoint = viewpoint;

  GM_VINF("Window", "Dispatching " << views.size() << " views");
  for (auto view : views)
    view->renderFullPipeline(settings);
}

END_NAMESPACE_GMGRAPHICS;
