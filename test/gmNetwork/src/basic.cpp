
#include <gmNetwork/PeersConnection.hh>
#include <gmNetwork/ExecutionSynchronization.hh>

#include <gmCore/Configuration.hh>

#include <memory>
#include <thread>

using namespace gramods;

struct Peer {

  Peer(int host_idx, int delay_ms, int &count)
    : delay_ms(delay_ms),
      count(count) {

    std::stringstream ss_xml;
    ss_xml << "<config>" << std::endl;
    ss_xml << "  <ExecutionSynchronization>" << std::endl;
    ss_xml << "    <PeersConnection" << std::endl;
    ss_xml << "        AS=\"connection\"" << std::endl;
    ss_xml << "        DEF=\"PEERS\"" << std::endl;
    ss_xml << "        localPeerIdx=\"" << host_idx << "\">" << std::endl;
    ss_xml << "      <param name=\"peer\" value=\"127.0.0.1:20402\"/>" << std::endl;
    ss_xml << "      <param name=\"peer\" value=\"127.0.0.1:20403\"/>" << std::endl;
    //ss_xml << "      <param name=\"peer\" value=\"127.0.0.1:20404\"/>" << std::endl;
    ss_xml << "    </PeersConnection>" << std::endl;
    ss_xml << "  </ExecutionSynchronization>" << std::endl;
    //ss_xml << "  <VariableSynchronization>" << std::endl;
    //ss_xml << "    <PeersConnection" << std::endl;
    //ss_xml << "        USE=\"PEERS\"/>" << std::endl;
    //ss_xml << "  </VariableSynchronization>" << std::endl;
    ss_xml << "</config>" << std::endl;

    gmCore::Configuration config(ss_xml.str());

    EXPECT_TRUE(config.getObject(sync));

    if (sync)
      thread = std::make_unique<std::thread>([this](){ this->run(); });
  }

  ~Peer() {
    {
      sync->getConnection()->close();
      std::lock_guard<std::mutex> guard(lock);
      sync = 0;
    }
    thread->join();
  }

  std::shared_ptr<gmNetwork::ExecutionSynchronization> sync;
  std::unique_ptr<std::thread> thread;
  std::mutex lock;
  int delay_ms;
  int &count;

  void run() {
    {
      std::lock_guard<std::mutex> guard(lock);
      sync->waitForConnection();
    }

    while (true) {
      std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));
      {
        std::lock_guard<std::mutex> guard(lock);
        if (!sync)
          break;
        sync->waitForAll();
      }
      ++count;
    }
  }
};

TEST(gmNetworkPeersConnection, waitForAll) {
  int count0 = 0;
  int count1 = 0;
  int count2 = 0;

  {
    std::vector<std::shared_ptr<Peer>> peers;
    peers.push_back(std::make_shared<Peer>(0, 1, std::ref(count0)));
    peers.push_back(std::make_shared<Peer>(1, 2, std::ref(count1)));
    //peers.push_back(std::make_shared<Peer>(2, 5, std::ref(count1)));

    std::this_thread::sleep_for(std::chrono::milliseconds(20));
  }

  EXPECT_GT(count0, 1);
  EXPECT_GT(count1, 1);
  //EXPECT_GT(count2, 0);

  EXPECT_EQ(count0, count1);
  //EXPECT_EQ(count1, count2);
  //EXPECT_EQ(count0, count2);
}
