
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
    ++count;
    netsync->waitForAll();
  }
}

TEST(gmNetworkNetSync, waitForAll) {
  int count0 = 0;
  int count1 = 0;
  int count2 = 0;
  bool alive = true;

  std::thread t0(NetSyncTest, std::string("127.1.1.2"), 1, std::ref(count0), std::ref(alive));
  std::thread t1(NetSyncTest, std::string("127.1.1.3"), 2, std::ref(count1), std::ref(alive));
  std::thread t2(NetSyncTest, std::string("127.1.1.4"), 5, std::ref(count2), std::ref(alive));

  std::this_thread::sleep_for(std::chrono::milliseconds(20));
  alive = false;

  t0.join();
  t1.join();
  t2.join();

  EXPECT_EQ(count0, count1);
  EXPECT_EQ(count1, count2);
  EXPECT_EQ(count0, count2);
}
