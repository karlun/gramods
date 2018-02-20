
#include <gmConfig/ImportLibrary.hh>

#include <memory>
#include <string>

using namespace gramods;

TEST(gmConfigLoadLib, SimpleLoad) {
  
  gmConfig::Configuration config("<config param=\"5\"><ImportLibrary file=\"test-lib\"/></config>");
  int param;
  EXPECT_TRUE(config.getParam("param", param));
  EXPECT_EQ(5, param);
  
  std::vector<std::shared_ptr<gmConfig::ImportLibrary>> importers;
  EXPECT_EQ(1, config.getAllObjects(importers));
  if (importers.size() == 1)
    EXPECT_TRUE(importers[0]->isLoaded());
}
