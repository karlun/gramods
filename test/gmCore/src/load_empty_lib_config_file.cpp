
#include <gmCore/ImportLibrary.hh>
#include <gmCore/Configuration.hh>

#include <memory>
#include <string>

using namespace gramods;

TEST(gmCoreLoadLib, SimpleLoad) {
  
  gmCore::Configuration config1("<config param=\"5\"><ImportLibrary file=\"test-lib\"><ImportLibrary AS=\"child\" file=\"test-lib\"/></ImportLibrary></config>");

  int param;
  EXPECT_TRUE(config1.getParam("param", param));
  EXPECT_EQ(5, param);
  
  std::vector<std::shared_ptr<gmCore::ImportLibrary>> importers;
  EXPECT_EQ(1, config1.getAllObjects(importers));
  for (auto importer : importers)
    EXPECT_TRUE(importer->isLoaded());

  if (importers.size() > 0) {
    EXPECT_TRUE(importers.front()->getChild());
    if (importers.front()->getChild())
      EXPECT_FALSE(importers.front()->getChild()->getChild());
  }
  
  ASSERT_THROW(gmCore::Configuration config2("<config param=\"5\"><ImportLibrary fileblob=\"test-lib\"/></config>"),
               std::invalid_argument);
}
