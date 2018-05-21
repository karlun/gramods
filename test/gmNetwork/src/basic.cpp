
#include <gmNetwork/NetSync.hh>

#include <gmCore/Configuration.hh>

#include <memory>
#include <thread>

using namespace gramods;

void NetSyncTest(std::string host, int delay_ms, int &count, bool &alive) {
  std::stringstream ss_xml;
  ss_xml << "<config>" << std::endl;
  ss_xml << "  <NetSync>" << std::endl;
  ss_xml << "    <param name=\"bind\" value=\"" << host << "\"/>" << std::endl;
  ss_xml << "    <param name=\"peer\" value=\"127.1.1.2\"/>" << std::endl;
  ss_xml << "    <param name=\"peer\" value=\"127.1.1.3\"/>" << std::endl;
  ss_xml << "    <param name=\"peer\" value=\"127.1.1.4\"/>" << std::endl;
  ss_xml << "  </NetSync>" << std::endl;
  ss_xml << "</config>" << std::endl;

  gmCore::Configuration config(ss_xml.str());

  std::shared_ptr<gmNetwork::NetSync> netsync;
  EXPECT_TRUE(config.getObject(netsync));

  netsync->waitForAll();
  while (alive) {
    std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));
    
  }
}

TEST(gmNetworkNetSync, waitForAll) {
  int count0 = 0;
  int count1 = 0;
  int count2 = 0;
  bool alive = true;

  std::thread t0(NetSyncTest, std::string("127.1.1.2"),  1, count0, alive);
  std::thread t1(NetSyncTest, std::string("127.1.1.3"),  3, count1, alive);
  std::thread t2(NetSyncTest, std::string("127.1.1.4"), 10, count2, alive);

  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  alive = false;

  t0.join();
  t1.join();
  t2.join();

  EXPECT_EQ(count0, count1);
  EXPECT_EQ(count1, count2);
  EXPECT_EQ(count0, count2);
}
