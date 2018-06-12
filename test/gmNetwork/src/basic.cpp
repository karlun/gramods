
#include <gmNetwork/NetSync.hh>

#include <gmCore/Configuration.hh>

#include <memory>
#include <thread>

using namespace gramods;

struct Peer {

  Peer(std::string host, int delay_ms, int &count)
    : delay_ms(delay_ms),
      count(count) {

    std::stringstream ss_xml;
    ss_xml << "<config>" << std::endl;
    ss_xml << "  <NetSync>" << std::endl;
    ss_xml << "    <param name=\"bind\" value=\"" << host << "\"/>" << std::endl;
    ss_xml << "    <param name=\"peer\" value=\"127.0.0.1:20402\"/>" << std::endl;
    ss_xml << "    <param name=\"peer\" value=\"127.0.0.1:20403\"/>" << std::endl;
    //ss_xml << "    <param name=\"peer\" value=\"127.0.0.1:20404\"/>" << std::endl;
    ss_xml << "  </NetSync>" << std::endl;
    ss_xml << "</config>" << std::endl;

    gmCore::Configuration config(ss_xml.str());

    EXPECT_TRUE(config.getObject(netsync));

    if (netsync)
      thread = std::make_unique<std::thread>([this](){ this->run(); });
  }

  ~Peer() {
    netsync = 0;
    thread->join();
  }

  std::shared_ptr<gmNetwork::NetSync> netsync;
  std::unique_ptr<std::thread> thread;
  int delay_ms;
  int &count;

  void run() {
    netsync->waitForConnection();

    while (netsync) {
      ++count;
      std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));
      netsync->waitForAll();
    }
  }
};

TEST(gmNetworkNetSync, waitForAll) {
  int count0 = 0;
  int count1 = 0;
  int count2 = 0;

  {
    std::vector<std::shared_ptr<Peer>> peers;
    peers.push_back(std::make_shared<Peer>(std::string("127.0.0.1:20402"),
                                           1, std::ref(count0)));
    peers.push_back(std::make_shared<Peer>(std::string("127.0.0.1:20403"),
                                           2, std::ref(count1)));
    /*peers.push_back(std::make_shared<Peer>(std::string("127.0.0.1:20404"),
      5, std::ref(count1), std::ref(alive)));*/

    std::this_thread::sleep_for(std::chrono::milliseconds(20));
  }

  EXPECT_GT(count0, 0);
  EXPECT_GT(count1, 0);
  EXPECT_GT(count2, 0);

  EXPECT_EQ(count0, count1);
  EXPECT_EQ(count1, count2);
  EXPECT_EQ(count0, count2);
}
