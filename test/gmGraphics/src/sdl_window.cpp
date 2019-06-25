
#include <gmCore/Configuration.hh>
#include <gmCore/Console.hh>
#include <gmCore/OStreamMessageSink.hh>
#include <gmCore/ImportLibrary.hh>
#include <gmCore/SdlContext.hh>
#include <gmGraphics/SdlWindow.hh>

#include <chrono>
#include <thread>

using namespace gramods;

#ifdef gramods_ENABLE_SDL2

TEST(gmGraphics, SdlWindow) {

  gmCore::ImportLibrary importer;
  importer.setLibrary("libgmGraphics.so");

  auto context = std::make_shared<gmCore::SdlContext>();
  context->setUseVideo(true);
  context->initialize();

  auto window = std::make_shared<gmGraphics::SdlWindow>();
  window->setContext(context);
  window->initialize();

  std::this_thread::sleep_for(std::chrono::seconds(1));
}

#endif

#ifdef gramods_ENABLE_TinyXML2
#ifdef gramods_ENABLE_SDL2

TEST(gmGraphics, SdlWindowXml) {

  std::string xml = ""
    "<config>"
    "  <ImportLibrary library=\"libgmGraphics.so\"/>"
    "  <SdlWindow size=\"100 100\">"
    "    <SdlContext AS=\"context\" useVideo=\"1\"/>"
    "  </SdlWindow>"
    "</config>";

  gmCore::Configuration config(xml);

  std::shared_ptr<gmGraphics::Window> win;
  config.getObject(win);
  ASSERT_TRUE(win);

  std::this_thread::sleep_for(std::chrono::seconds(1));
}

#endif
#endif
