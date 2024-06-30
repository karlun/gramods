
#include <gmGraphics/Window.hh>
#include <gmGraphics/View.hh>

#include <GL/glew.h>
#include <GL/gl.h>

BEGIN_NAMESPACE_GMGRAPHICS;

GM_OFI_DEFINE_ABSTRACT_SUB(Window, ViewBase);
GM_OFI_PARAM2(Window, fullscreen, bool, setFullscreen);
GM_OFI_PARAM2(Window, display, size_t, setDisplay);
GM_OFI_PARAM2(Window, title, std::string, setTitle);
GM_OFI_PARAM2(Window, size, gmCore::size2, setSize);
GM_OFI_PARAM2(Window, position, gmCore::int2, setPosition);
GM_OFI_PARAM2(Window, backgroundColor, gmCore::float4, setBackgroundColor);
GM_OFI_POINTER2(Window, view, gmGraphics::View, addView);

Window::Window() {}

void Window::renderFullPipeline(ViewSettings settings) {
  populateViewSettings(settings);

  makeGLContextCurrent();
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glViewport(0, 0, getSize()[0], getSize()[1]);

  glClearColor(background_color[0],
               background_color[1],
               background_color[2],
               background_color[3]);
  glClear(GL_COLOR_BUFFER_BIT |
          GL_DEPTH_BUFFER_BIT);

  if (!views.empty()) {

    for (auto view : views)
      view->renderFullPipeline(settings);

  } else {

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    Camera c(settings);
    settings.renderNodes(c);
  }
  glFlush();
}

bool Window::handleEvent(event *evt) {
  for (auto fun : event_handlers)
    if (fun.second(evt)) return true;
  return false;
}

void Window::addEventHandler(std::function<bool(const event*)> fun, void *tag) {
  if (event_handlers.count(tag) > 0)
    throw gmCore::InvalidArgument("cannot associate tag with more than one event handler.");
  event_handlers[tag] = fun;
}

void Window::removeEventHandler(void *tag) {
  if (event_handlers.count(tag) <= 0) return;
  event_handlers.erase(tag);
}

void Window::sync() {
  makeGLContextCurrent();
  glFinish();
}

void Window::traverse(Visitor *visitor) {
  ViewBase::traverse(visitor);
  for (auto &v : views) v->accept(visitor);
}

END_NAMESPACE_GMGRAPHICS;
