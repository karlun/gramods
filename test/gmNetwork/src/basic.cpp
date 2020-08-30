
#include <gmNetwork/PeersConnection.hh>
#include <gmNetwork/ExecutionSynchronization.hh>
#include <gmNetwork/SyncSData.hh>
#include <gmNetwork/SyncMData.hh>
#include <gmNetwork/SimpleDataSynchronization.hh>

#include <gmCore/Console.hh>
#include <gmCore/OStreamMessageSink.hh>

#include <memory>
#include <thread>

using namespace gramods;

#if 1
struct Peer {

  Peer(int host_idx, int delay_ms)
    : delay_ms(delay_ms) {

    auto conn = std::make_shared<gmNetwork::PeersConnection>();
    conn->addPeer("127.0.0.1:20400");
    conn->addPeer("127.0.0.1:20401");
    conn->addPeer("127.0.0.1:20402");
    conn->setLocalPeerIdx(host_idx);
    conn->initialize();

    exec_sync = std::make_shared<gmNetwork::ExecutionSynchronization>();
    exec_sync->setConnection(conn);
    exec_sync->initialize();

    data_sync = std::make_shared<gmNetwork::SimpleDataSynchronization>();
    data_sync->setConnection(conn);
    data_sync->initialize();

    data_int64 = std::make_shared<gmNetwork::SyncSInt64>();
    data_float64 = std::make_shared<gmNetwork::SyncSFloat64>();
    data_vec_float64 = std::make_shared<gmNetwork::SyncMFloat64>();

    data_sync->addData(&data_bool);
    data_sync->addData(&data_vec_bool);
    data_sync->addData(&data_int32);
    data_sync->addData(&data_float32);
    data_sync->addData(data_int64);
    data_sync->addData(data_float64);
    data_sync->addData(data_vec_float64);
    is_primary = data_sync->getConnection()->getLocalPeerIdx() == 0;

    thread = std::make_unique<std::thread>([this](){ this->run(); });
  }

  ~Peer() {
    {
      std::lock_guard<std::mutex> guard(sync_lock);
      exec_sync->getConnection()->close();
      exec_sync = 0;
      data_sync = 0;
    }
    thread->join();
  }

  bool is_primary = false;
  int state = 0;

  std::shared_ptr<gmNetwork::ExecutionSynchronization> exec_sync;
  std::shared_ptr<gmNetwork::SimpleDataSynchronization> data_sync;

  int count = 0;

  gmNetwork::SyncSBool data_bool = false;
  gmNetwork::SyncSInt32 data_int32 = 0;
  gmNetwork::SyncSFloat32 data_float32 = 0;
  std::shared_ptr<gmNetwork::SyncSInt64> data_int64 = 0;
  std::shared_ptr<gmNetwork::SyncSFloat64> data_float64 = 0;

  gmNetwork::SyncMBool data_vec_bool;
  std::shared_ptr<gmNetwork::SyncMFloat64> data_vec_float64;

  std::unique_ptr<std::thread> thread;
  std::mutex sync_lock;
  std::mutex count_lock;
  int delay_ms;

  void run() {
    {
      std::lock_guard<std::mutex> guard(sync_lock);
      exec_sync->waitForConnection();
    }

    if (is_primary) {
      data_bool = true;
      data_int32 = std::numeric_limits<int32_t>::max() - 41;
      *data_int64 = std::numeric_limits<int64_t>::max() - 97;
      data_float32 = 1.f / 3.f;
      *data_float64 = 1.0 / 6.0;

      std::vector<char> vec_bool = data_vec_bool;
      vec_bool.push_back(false);
      vec_bool.push_back(true);
      vec_bool.push_back(false);
      data_vec_bool = vec_bool;
    }

    while (true) {
      std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));
      {
        std::shared_ptr<gmNetwork::ExecutionSynchronization> tmp_exec_sync;
        {
          std::lock_guard<std::mutex> guard(sync_lock);
          tmp_exec_sync = exec_sync;
        }
        if (state != 0) break;
        tmp_exec_sync->waitForAll();
      }

      {
        std::lock_guard<std::mutex> guard(count_lock);
        ++count;
      }
    }

    data_sync->update();
    state = 2;
  }
};

TEST(gmNetworkPeersConnection, waitForAll) {

  gmCore::Console::removeAllSinks();
#if 0
  std::shared_ptr<gmCore::OStreamMessageSink> osms =
    std::make_shared<gmCore::OStreamMessageSink>();
  osms->setUseAnsiColor(false);
  osms->initialize();
#endif

  std::vector<std::shared_ptr<Peer>> peers;
  peers.push_back(std::make_shared<Peer>(0, 1));
  peers.push_back(std::make_shared<Peer>(1, 2));
  peers.push_back(std::make_shared<Peer>(2, 5));

  while (true) {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    {
      std::lock_guard<std::mutex> guard(peers.back()->count_lock);
      if (peers.back()->count > 100) {
        for (auto peer : peers) {
          peer->state = 1;
          peer->exec_sync->cancelWait();
        }
        break;
      }
    }
  }

  EXPECT_FALSE(peers[0]->data_bool);
  EXPECT_FALSE(peers[1]->data_bool);
  EXPECT_FALSE(peers[2]->data_bool);

  while (true) {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    {
      std::lock_guard<std::mutex> guard(peers.back()->count_lock);
      if (peers.front()->state == 2)
        break;
    }
  }

  EXPECT_GT(peers[0]->count, 10);
  EXPECT_GT(peers[1]->count, 10);
  EXPECT_GT(peers[2]->count, 10);

  EXPECT_LE(abs(peers[0]->count - peers[1]->count), 1);
  EXPECT_LE(abs(peers[0]->count - peers[2]->count), 1);
  EXPECT_LE(abs(peers[1]->count - peers[2]->count), 1);

  EXPECT_TRUE(peers[0]->data_bool);
  EXPECT_TRUE(peers[1]->data_bool);
  EXPECT_TRUE(peers[2]->data_bool);

  EXPECT_EQ(peers[0]->data_int32, peers[1]->data_int32);
  EXPECT_EQ(peers[0]->data_int32, peers[2]->data_int32);

  EXPECT_EQ(*peers[0]->data_int64, *peers[1]->data_int64);
  EXPECT_EQ(*peers[0]->data_int64, *peers[2]->data_int64);

  EXPECT_EQ(peers[0]->data_float32, peers[1]->data_float32);
  EXPECT_EQ(peers[0]->data_float32, peers[2]->data_float32);

  EXPECT_EQ(*peers[0]->data_float64, *peers[1]->data_float64);
  EXPECT_EQ(*peers[0]->data_float64, *peers[2]->data_float64);

  std::vector<char> vec_bool0 = peers[0]->data_vec_bool;
  std::vector<char> vec_bool1 = peers[1]->data_vec_bool;
  std::vector<char> vec_bool2 = peers[2]->data_vec_bool;

  EXPECT_GT(vec_bool0.size(), 1);
  EXPECT_EQ(vec_bool0.size(), vec_bool1.size());
  EXPECT_EQ(vec_bool0.size(), vec_bool2.size());

  if (vec_bool0.size() == vec_bool1.size() &&
      vec_bool0.size() == vec_bool2.size())
    for (size_t idx = 0; idx < vec_bool0.size(); ++idx) {
      EXPECT_EQ(vec_bool0[idx], vec_bool1[idx]);
      EXPECT_EQ(vec_bool0[idx], vec_bool2[idx]);
    }
}

#endif
