
#include <gmTypes/config.hh>

#include <gmTypes/angle.hh>

#include <gmCore/Console.hh>
#include <gmCore/OStreamMessageSink.hh>

#include <memory>
#include <sstream>

using namespace gramods;

TEST(gmTypesAngle, ReadFloat) {

  std::stringstream out;

  std::shared_ptr<gmCore::OStreamMessageSink> osms =
    std::make_shared<gmCore::OStreamMessageSink>();
  osms->setStream(&out);
  osms->setLevel(1);
  osms->initialize();

  {
    std::stringstream ss("180");
    gmTypes::angle a;
    ss >> a;

    EXPECT_TRUE(ss);
    EXPECT_LE(fabsf(180 - a), std::numeric_limits<float>::epsilon());

    EXPECT_FALSE(out.str().find("'d180'") == std::string::npos);
    EXPECT_FALSE(out.str().find("'r180'") == std::string::npos);
  }

  gmCore::Console::removeAllSinks();
}

TEST(gmTypesAngle, ReadRadians) {

  std::stringstream out;

  std::shared_ptr<gmCore::OStreamMessageSink> osms =
    std::make_shared<gmCore::OStreamMessageSink>();
  osms->setStream(&out);
  osms->setLevel(1);
  osms->initialize();

  {
    std::stringstream ss("r180");
    gmTypes::angle a;
    ss >> a;
    EXPECT_TRUE(ss);
    EXPECT_LE(fabsf(180 - a), std::numeric_limits<float>::epsilon());

    EXPECT_TRUE(out.str().empty());
  }

  gmCore::Console::removeAllSinks();
}

TEST(gmTypesAngle, ReadDegrees) {

  std::stringstream out;

  std::shared_ptr<gmCore::OStreamMessageSink> osms =
    std::make_shared<gmCore::OStreamMessageSink>();
  osms->setStream(&out);
  osms->setLevel(1);
  osms->initialize();

  {
    std::stringstream ss("d180");
    gmTypes::angle a;
    ss >> a;
    EXPECT_TRUE(ss);
    EXPECT_LE(fabsf(gramods_PI - a), std::numeric_limits<float>::epsilon());

    EXPECT_TRUE(out.str().empty());
  }

  gmCore::Console::removeAllSinks();
}

TEST(gmTypesAngle, ReadNegativeDegrees) {

  {
    std::stringstream ss("d-180");
    gmTypes::angle a;
    ss >> a;
    EXPECT_TRUE(ss);
    EXPECT_LE(fabsf(gramods_PI + a), std::numeric_limits<float>::epsilon());
  }
}

TEST(gmTypesAngle, ReadWrongKey) {

  std::stringstream out;

  std::shared_ptr<gmCore::OStreamMessageSink> osms =
    std::make_shared<gmCore::OStreamMessageSink>();
  osms->setStream(&out);
  osms->setLevel(1);
  osms->initialize();

  {
    std::stringstream ss("q180");
    gmTypes::angle a;
    ss >> a;
    EXPECT_FALSE(ss);

    EXPECT_FALSE(out.str().find("'q180'") == std::string::npos);
  }

  gmCore::Console::removeAllSinks();
}

TEST(gmTypesAngle, ReadMultiple) {

  std::stringstream out;

  std::shared_ptr<gmCore::OStreamMessageSink> osms =
    std::make_shared<gmCore::OStreamMessageSink>();
  osms->setStream(&out);
  osms->setLevel(1);
  osms->initialize();

  {
    std::stringstream ss("r180 d180 3");
    gmTypes::angle a, b, c;
    ss >> a >> b >> c;
    EXPECT_TRUE(ss);
    EXPECT_LE(fabsf(180 - a), std::numeric_limits<float>::epsilon());
    EXPECT_LE(fabsf(gramods_PI - b), std::numeric_limits<float>::epsilon());
    EXPECT_LE(fabsf(3 - c), std::numeric_limits<float>::epsilon());

    EXPECT_TRUE(out.str().empty());
  }

  gmCore::Console::removeAllSinks();
}
