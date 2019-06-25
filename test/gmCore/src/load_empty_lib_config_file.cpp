
#include <gmCore/ImportLibrary.hh>
#include <gmCore/Configuration.hh>

#include <memory>
#include <string>

using namespace gramods;

TEST(gmCoreLoadLib, SimpleLoad) {

  auto importer = std::make_shared<gmCore::ImportLibrary>();
  importer->setLibrary("test-lib");
  importer->initialize();

  EXPECT_FALSE(importer->isLoaded());
}

#ifdef gramods_ENABLE_TinyXML2

TEST(gmCoreLoadLib, SimpleLoadXml) {

  std::string xml = R"lang=xml(
  <config>
    <ImportLibrary library="test-lib"/>
  </config>
  )lang=xml";

  gmCore::Configuration config(xml);

  std::vector<std::shared_ptr<gmCore::ImportLibrary>> importers;
  EXPECT_EQ(1, config.getAllObjects(importers));
  for (auto importer : importers)
    EXPECT_FALSE(importer->isLoaded());
}

#endif
