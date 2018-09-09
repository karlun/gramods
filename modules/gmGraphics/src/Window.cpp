
#include <gmGraphics/SdlWindow.hh>

BEGIN_NAMESPACE_GMGRAPHICS;

GM_OFI_DEFINE(Window);
GM_OFI_PARAM(Window, fullscreen, bool, Window::setFullscreen);
GM_OFI_PARAM(Window, title, std::string, Window::setTitle);
GM_OFI_PARAM(Window, size, gmTypes::size2, Window::setSize);

Window::Window()
  : fullscreen(false),
    title("untitled gramods window"),
    size(gmTypes::size2({1920, 1080})) {}

END_NAMESPACE_GMGRAPHICS;
