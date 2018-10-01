
#include <gmGraphics/Window.hh>
#include <gmGraphics/View.hh>

#include <GL/glew.h>
#include <GL/gl.h>

BEGIN_NAMESPACE_GMGRAPHICS;

GM_OFI_DEFINE_SUB(Window, RendererDispatcher);
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

  settings.renderers.insert(settings.renderers.end(),
                            renderers.begin(), renderers.end());

  if (viewpoint)
    settings.viewpoint = viewpoint;

  glViewport(0, 0, getSize()[0], getSize()[1]);

  if (!views.empty()) {

    for (auto view : views)
      view->renderFullPipeline(settings);

  } else {

    Camera c;
    for (auto renderer : settings.renderers)
      renderer->render(c);
  }
}

END_NAMESPACE_GMGRAPHICS;
