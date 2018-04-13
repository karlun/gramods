
#include <gmCore/Console.hh>
#include <gmCore/OStreamMessageSink.hh>
#include <gmCore/Configuration.hh>

#include <memory>
#include <string>
#include <sstream>
#include <regex>

using namespace gramods;

std::string removePath(std::string data) {
  std::regex expr("\\s([^ ]*)(.test.gmCore.src.)(console\\.cpp)");
  std::string result;
  std::regex_replace (std::back_inserter(result),
                      data.begin(), data.end(),
                      expr, " /PATH/$3");
  return result;
}

TEST(gmCoreConsole, OStreamMessageSink_sstream) {

  gmCore::Console::setDefaultSink(nullptr);

  std::stringstream ss;

  std::shared_ptr<gmCore::OStreamMessageSink> osms =
    std::make_shared<gmCore::OStreamMessageSink>();
  osms->setStream(&ss);
  osms->initialize();

  GM_INF("a", "A");
  GM_WRN("b", "B");
  GM_ERR("c", "C");
  GM_VINF("d", "E");
  GM_VVINF("e", "E");

  std::string result = removePath(ss.str());

  EXPECT_EQ(std::string("II (a) /PATH/console.cpp:33 (TestBody)\n"
                        "II (a) A\n\n"
                        "WW (b) /PATH/console.cpp:34 (TestBody)\n"
                        "WW (b) B\n\n"
                        "EE (c) /PATH/console.cpp:35 (TestBody)\n"
                        "EE (c) C\n\n"
                        "I2 (d) /PATH/console.cpp:36 (TestBody)\n"
                        "I2 (d) E\n\n"
                        "I3 (e) /PATH/console.cpp:37 (TestBody)\n"
                        "I3 (e) E\n\n"), result);
}

class ScopedRedirect {
public:
  ScopedRedirect(std::ostream & inOriginal, std::ostream & inRedirect) :
    mOriginal(inOriginal),
    mOldBuffer(inOriginal.rdbuf(inRedirect.rdbuf())) {}
  ~ScopedRedirect() {
    mOriginal.rdbuf(mOldBuffer);
  }    
private:
  ScopedRedirect(const ScopedRedirect&);
  ScopedRedirect& operator=(const ScopedRedirect&);

  std::ostream & mOriginal;
  std::streambuf * mOldBuffer;
};

TEST(gmCoreConsole, OStreamMessageSink_stdcout) {

  gmCore::Console::setDefaultSink(nullptr);

  std::string xml = ""
    "<config>"
    "  <OStreamMessageSink stream=\"out\"/>"
    "</config>";

  std::stringstream ss;
  {
    ScopedRedirect redirect(std::cout, ss);

    gmCore::Configuration config(xml);

    ss.clear();

    GM_INF("a", "A");
  }

  std::string result = removePath(ss.str());
  EXPECT_EQ(std::string("II (a) /PATH/console.cpp:86 (TestBody)\n"
                        "II (a) A\n\n"), result);
}
