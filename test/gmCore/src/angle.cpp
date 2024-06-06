
#include <gmCore/config.hh>

#include <gmCore/io_angle.hh>

#include <gmCore/Console.hh>
#include <gmCore/OStreamMessageSink.hh>

#include <math.h>

#include <memory>
#include <sstream>

using namespace gramods;

TEST(gmCoreAngle, ReadFloat) {

  std::stringstream out;

  std::shared_ptr<gmCore::OStreamMessageSink> osms =
    std::make_shared<gmCore::OStreamMessageSink>();
  osms->setStream(&out);
  osms->setLevel(1);
  osms->initialize();

  {
    std::stringstream ss("180");
    gmCore::angle a;
    ss >> a;

    EXPECT_TRUE(ss);
    EXPECT_FLOAT_EQ(a, 180.f);

    EXPECT_FALSE(out.str().find("'d180'") == std::string::npos);
    EXPECT_FALSE(out.str().find("'r180'") == std::string::npos);
  }

  gmCore::Console::removeAllSinks();
}

TEST(gmCoreAngle, ReadRadians) {

  std::stringstream out;

  std::shared_ptr<gmCore::OStreamMessageSink> osms =
    std::make_shared<gmCore::OStreamMessageSink>();
  osms->setStream(&out);
  osms->setLevel(1);
  osms->initialize();

  {
    std::stringstream ss("r180");
    gmCore::angle a;
    ss >> a;
    EXPECT_TRUE(ss);
    EXPECT_FLOAT_EQ(a, 180.f);

    EXPECT_TRUE(out.str().empty());
  }

  gmCore::Console::removeAllSinks();
}

TEST(gmCoreAngle, ReadDegrees) {

  std::stringstream out;

  std::shared_ptr<gmCore::OStreamMessageSink> osms =
    std::make_shared<gmCore::OStreamMessageSink>();
  osms->setStream(&out);
  osms->setLevel(1);
  osms->initialize();

  {
    std::stringstream ss("d180");
    gmCore::angle a;
    ss >> a;
    EXPECT_TRUE(ss);
    EXPECT_FLOAT_EQ(a, GM_PI);

    EXPECT_TRUE(out.str().empty());
  }

  gmCore::Console::removeAllSinks();
}

TEST(gmCoreAngle, ReadNegativeDegrees) {

  {
    std::stringstream ss("d-180");
    gmCore::angle a;
    ss >> a;
    EXPECT_TRUE(ss);
    EXPECT_FLOAT_EQ(a, -GM_PI);
  }
}

TEST(gmCoreAngle, ReadWrongKey) {

  std::stringstream out;

  std::shared_ptr<gmCore::OStreamMessageSink> osms =
    std::make_shared<gmCore::OStreamMessageSink>();
  osms->setStream(&out);
  osms->setLevel(1);
  osms->initialize();

  {
    std::stringstream ss("q180");
    gmCore::angle a;
    ss >> a;
    EXPECT_FALSE(ss);

    EXPECT_FALSE(out.str().find("'q180'") == std::string::npos);
  }

  gmCore::Console::removeAllSinks();
}

TEST(gmCoreAngle, ReadMultiple) {

  std::stringstream out;

  std::shared_ptr<gmCore::OStreamMessageSink> osms =
    std::make_shared<gmCore::OStreamMessageSink>();
  osms->setStream(&out);
  osms->setLevel(1);
  osms->initialize();

  {
    std::stringstream ss("r180 d180 3");
    gmCore::angle a, b, c;
    ss >> a >> b >> c;
    EXPECT_TRUE(ss);
    EXPECT_FLOAT_EQ(a, 180.f);
    EXPECT_FLOAT_EQ(b, GM_PI);
    EXPECT_FLOAT_EQ(c, 3.f);

    EXPECT_TRUE(out.str().empty());
  }

  gmCore::Console::removeAllSinks();
}
