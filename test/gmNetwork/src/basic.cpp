
#include <gmNetwork/PeersConnection.hh>
#include <gmNetwork/ExecutionSynchronization.hh>

#include <gmCore/Console.hh>
#include <gmCore/OStreamMessageSink.hh>
#include <gmCore/Configuration.hh>

#include <memory>
#include <thread>

using namespace gramods;

struct Peer {

  Peer(int host_idx, int delay_ms, int &count)
    : delay_ms(delay_ms),
      count(count) {

    std::string xml_tmpl = R"lang=xml(
<config>
  <ExecutionSynchronization>
    <PeersConnection
        DEF="PEERS"
        localPeerIdx="%d">
      <param name="peer" value="127.0.0.1:20400"/>
      <param name="peer" value="127.0.0.1:20401"/>
      <param name="peer" value="127.0.0.1:20402"/>
    </PeersConnection>
  </ExecutionSynchronization>
  <!--VariableSynchronization>
    <PeersConnection
        USE="PEERS"/>
  </VariableSynchronization-->
</config>
)lang=xml";

    size_t xml_size = snprintf(nullptr, 0, xml_tmpl.c_str(), host_idx) + 1;
    std::vector<char> xml_str(xml_size);
    snprintf(&xml_str[0], xml_size, xml_tmpl.c_str(), host_idx);

    gmCore::Configuration config(&xml_str[0]);

    EXPECT_TRUE(config.getObject(sync));

    if (sync)
      thread = std::make_unique<std::thread>([this](){ this->run(); });
  }

  ~Peer() {
    {
      std::lock_guard<std::mutex> guard(sync_lock);
      sync->getConnection()->close();
      sync = 0;
    }
    thread->join();
  }

  std::shared_ptr<gmNetwork::ExecutionSynchronization> sync;
  std::unique_ptr<std::thread> thread;
  std::mutex sync_lock;
  std::mutex count_lock;
  int delay_ms;
  int &count;

  void run() {
    {
      std::lock_guard<std::mutex> guard(sync_lock);
      sync->waitForConnection();
    }

    while (true) {
      std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));
      {
        std::shared_ptr<gmNetwork::ExecutionSynchronization> tmp_sync;
        {
          std::lock_guard<std::mutex> guard(sync_lock);
          tmp_sync = sync;
        }
        if (!tmp_sync) break;
        tmp_sync->waitForAll();
      }

      {
        std::lock_guard<std::mutex> guard(count_lock);
        ++count;
      }
    }
  }
};

TEST(gmNetworkPeersConnection, waitForAll) {

  gmCore::Console::removeAllSinks();
#if 0
  std::shared_ptr<gmCore::OStreamMessageSink> osms =
    std::make_shared<gmCore::OStreamMessageSink>();
  osms->initialize();
#endif

  int count0 = 0;
  int count1 = 0;
  int count2 = 0;

  {
    std::vector<std::shared_ptr<Peer>> peers;
    peers.push_back(std::make_shared<Peer>(0, 1, std::ref(count0)));
    peers.push_back(std::make_shared<Peer>(1, 2, std::ref(count1)));
    peers.push_back(std::make_shared<Peer>(2, 5, std::ref(count2)));

    while (true) {
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
      {
        std::lock_guard<std::mutex> guard(peers.back()->count_lock);
        if (peers.back()->count > 100)
          break;
      }
    }
  }

  EXPECT_GT(count0, 10);
  EXPECT_GT(count1, 10);
  EXPECT_GT(count2, 10);

  EXPECT_LE(abs(count0 - count1), 1);
  EXPECT_LE(abs(count0 - count2), 1);
  EXPECT_LE(abs(count1 - count2), 1);
}
