
#include <gmGraphics/Window.hh>
#include <gmCore/Configuration.hh>
#include <gmCore/Console.hh>
#include <gmCore/OStreamMessageSink.hh>
#include <gmCore/ImportLibrary.hh>

#include <chrono>
#include <thread>

using namespace gramods;

TEST(gmCore, ImportLibrary) {
  std::string xml = ""
    "<config>"
    "  <ImportLibrary lib=\"libgmGraphics.so\"/>"
    "</config>";
  gmCore::Configuration config(xml);

  std::vector<std::shared_ptr<gmCore::ImportLibrary>> importers;
  EXPECT_EQ(1, config.getAllObjects(importers));

  if (!importers.empty())
    EXPECT_TRUE(importers[0]->isLoaded());
}

#ifdef gramods_ENABLE_SDL2

TEST(gmGraphics, SdlWindow) {

  std::string xml = ""
    "<config>"
    "  <ImportLibrary lib=\"libgmGraphics.so\"/>"
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
