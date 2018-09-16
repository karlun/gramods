
#include <gmCore/ImportLibrary.hh>
#include <gmCore/Configuration.hh>

#include <memory>
#include <string>

using namespace gramods;

TEST(gmCoreLoadLib, SimpleLoad) {

  std::string xml = ""
    "<config>"
    "  <ImportLibrary lib=\"test-lib\"/>"
    "</config>";
  gmCore::Configuration config(xml);

  std::vector<std::shared_ptr<gmCore::ImportLibrary>> importers;
  EXPECT_EQ(1, config.getAllObjects(importers));
  for (auto importer : importers)
    EXPECT_FALSE(importer->isLoaded());
}
