
#include <gmIO/Console.hh>
#include <gmIO/OStreamMessageSink.hh>

#include <gmConfig/Configuration.hh>

#include <memory>
#include <string>
#include <sstream>
#include <regex>

using namespace gramods;

std::string removePath(std::string data) {
  std::regex expr("\\s([^ ]*)(.test.gmIO.src.)(console\\.cpp)");
  std::string result;
  std::regex_replace (std::back_inserter(result),
                      data.begin(), data.end(),
                      expr, " /PATH/$3");
  return result;
}

TEST(gmIOConsole, OStreamMessageSink_sstream) {

  std::stringstream ss;

  std::shared_ptr<gmIO::OStreamMessageSink> osms =
    std::make_shared<gmIO::OStreamMessageSink>();
  osms->setStream(&ss);
  osms->initialize();

  GM_INF("a", "A");
  GM_WRN("b", "B");
  GM_ERR("c", "C");
  GM_VINF("d", "E");
  GM_VVINF("e", "E");

  std::string result = removePath(ss.str());

  EXPECT_EQ(std::string("II (a) /PATH/console.cpp:32 (TestBody)\n"
                        "II (a) A\n\n"
                        "WW (b) /PATH/console.cpp:33 (TestBody)\n"
                        "WW (b) B\n\n"
                        "EE (c) /PATH/console.cpp:34 (TestBody)\n"
                        "EE (c) C\n\n"
                        "I2 (d) /PATH/console.cpp:35 (TestBody)\n"
                        "I2 (d) E\n\n"
                        "I3 (e) /PATH/console.cpp:36 (TestBody)\n"
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

TEST(gmIOConsole, OStreamMessageSink_stdcout) {


  std::string xml = ""
    "<config>"
    "  <OStreamMessageSink stream=\"out\"/>"
    "</config>";

  gmConfig::Configuration config(xml);

  std::stringstream ss;
  {
    ScopedRedirect redirect(std::cout, ss);
    GM_INF("a", "A");
  }

  std::string result = removePath(ss.str());
  EXPECT_EQ(std::string("II (a) /PATH/console.cpp:81 (TestBody)\n"
                        "II (a) A\n\n"), result);
}
