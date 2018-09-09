
#include <gmGraphics/SdlWindow.hh>
#include <gmCore/Configuration.hh>
#include <gmCore/Console.hh>
#include <gmCore/OStreamMessageSink.hh>

#include <chrono>
#include <thread>

using namespace gramods;

TEST(gmGraphics, SdlWindow) {

  std::shared_ptr<gmCore::OStreamMessageSink> osms =
    std::make_shared<gmCore::OStreamMessageSink>();
  osms->initialize();

  std::string xml = ""
    "<config>"
    "  <SdlWindow size=\"100 100\">"
    "    <SdlContext AS=\"context\" useVideo=\"1\"/>"
    "  </SdlWindow>"
    "</config>";
  gmCore::Configuration config(xml);

  std::shared_ptr<gmGraphics::Window> win;
  config.getObject(win);

  std::this_thread::sleep_for(std::chrono::seconds(10));
}
