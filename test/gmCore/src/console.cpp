
#include <gmCore/Console.hh>
#include <gmCore/OStreamMessageSink.hh>
#include <gmCore/Configuration.hh>
#include <gmCore/ScopedOStreamRedirect.hh>

#include <memory>
#include <string>
#include <sstream>
#include <regex>

using namespace gramods;

TEST(gmCoreConsole, PathStripping) {
  constexpr auto filename = gmCore::detail::strip_path(__FILE__);
  constexpr auto basename = gmCore::detail::basename_impl
    (filename, gmCore::detail::last_dot_of(filename));

  EXPECT_EQ(std::string("console.cpp"), filename);
  EXPECT_EQ(std::string("console"), to_string(basename));
}

TEST(gmCoreConsole, OStreamMessageSink_sstream) {

  gmCore::Console::removeAllSinks();

  std::stringstream ss;

  std::shared_ptr<gmCore::OStreamMessageSink> osms =
    std::make_shared<gmCore::OStreamMessageSink>();
  osms->setStream(&ss);
  osms->initialize();

  int base_row = __LINE__;
  GM_INF("a", "A");
  GM_WRN("b", "B");
  GM_ERR("c", "C");
  GM_VINF("d", "E");
  GM_VVINF("e", "E");

  std::stringstream result;
  result << "II (a) console.cpp:" << base_row + 1 << " (TestBody)" << std::endl;
  result << "II (a) A" << std::endl;
  result << "WW (b) console.cpp:" << base_row + 2 << " (TestBody)" << std::endl;
  result << "WW (b) B" << std::endl;
  result << "EE (c) console.cpp:" << base_row + 3 << " (TestBody)" << std::endl;
  result << "EE (c) C" << std::endl;
  result << "I2 (d) console.cpp:" << base_row + 4 << " (TestBody)" << std::endl;
  result << "I2 (d) E" << std::endl;
  result << "I3 (e) console.cpp:" << base_row + 5 << " (TestBody)" << std::endl;
  result << "I3 (e) E" << std::endl;

  EXPECT_EQ(result.str(), ss.str());
}

TEST(gmCoreConsole, OStreamMessageSink_stdcout) {

  gmCore::Console::removeAllSinks();

  std::string xml = ""
    "<config>"
    "  <OStreamMessageSink stream=\"out\"/>"
    "</config>";

  std::stringstream ss;
  int base_row;
  {
    gmCore::ScopedOStreamRedirect redirect(std::cout, ss);

    gmCore::Configuration config(xml);

    ss.clear();

    base_row = __LINE__;
    GM_INF("a", "A");
  }

  std::stringstream result;
  result << "II (a) console.cpp:" << base_row + 1 << " (TestBody)" << std::endl;
  result << "II (a) A" << std::endl;

  EXPECT_EQ(result.str(), ss.str());

  gmCore::Console::removeAllSinks();
}
