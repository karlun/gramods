
#include <gmGraphics/SdlWindow.hh>

BEGIN_NAMESPACE_GMGRAPHICS;

GM_OFI_DEFINE(Window);
GM_OFI_PARAM(Window, fullscreen, bool, Window::setFullscreen);
GM_OFI_PARAM(Window, title, std::string, Window::setTitle);

END_NAMESPACE_GMGRAPHICS;
