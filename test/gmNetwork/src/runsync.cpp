
#include <gmNetwork/SyncNode.hh>
#include <gmNetwork/RunSync.hh>

#include <gmCore/Console.hh>
#include <gmCore/OStreamMessageSink.hh>
#include <gmCore/NullMessageSink.hh>
#include <gmCore/LogFileMessageSink.hh>

#include <atomic>
#include <memory>
#include <thread>
#include <sstream>
#include <random>

#define PORT0 25040
#define PORT1 26040

using namespace gramods;

TEST(gmNetwork, SyncNode_createdestroy) {

  gmCore::Console::removeAllSinks();
#if 0
  std::shared_ptr<gmCore::OStreamMessageSink> osms =
    std::make_shared<gmCore::OStreamMessageSink>();
  osms->setUseAnsiColor(true);
  osms->setLevel(2);
  osms->initialize();
#else
  std::shared_ptr<gmCore::NullMessageSink> nullsink =
    std::make_shared<gmCore::NullMessageSink>();
  nullsink->initialize();
#endif

  std::shared_ptr<gmNetwork::SyncNode> node =
    std::make_shared<gmNetwork::SyncNode>();
  node->addPeer("127.0.0.1:24040");
  node->setLocalPeerIdx(0);
  gmNetwork::RunSync * run =
    node->getProtocol<gmNetwork::RunSync>();
  EXPECT_TRUE(run);
}

namespace {

  void run_node(size_t idx, size_t peer_count,
                std::shared_ptr<std::atomic<size_t>> count,
                std::shared_ptr<std::atomic<bool>> run,
                std::shared_ptr<std::atomic<bool>> done) {

    static std::mutex random_lock;
    random_lock.lock();
    static std::default_random_engine generator
      (51254324234U);
    static std::uniform_real_distribution<double> distribution(0, 1);
    random_lock.unlock();

    std::shared_ptr<gmNetwork::SyncNode> node =
      std::make_shared<gmNetwork::SyncNode>();

    for (size_t port = PORT0; port < PORT0 + peer_count; ++port) {
      std::stringstream ss;
      ss << "127.0.0.1:" << port;
      node->addPeer(ss.str());
    }
    node->setLocalPeerIdx(idx);
    node->initialize();

    gmNetwork::RunSync * run_sync =
      node->getProtocol<gmNetwork::RunSync>();
    ASSERT_TRUE(run_sync);

    GM_DBG1("gTest", "Node " << idx << " waiting for connection");
    node->waitForConnection();

    double sleep_ratio;

    while (*run) {

      GM_DBG2("gTest", "Node " << idx << " calling wait");
      run_sync->wait();

      {
        std::unique_lock<std::mutex> locker(random_lock);
        sleep_ratio = distribution(generator);
      }
      std::this_thread::sleep_for(std::chrono::microseconds(int(1000 * sleep_ratio)));
      ++*count;
    }
    GM_DBG2("gTest", "Node " << idx << " done");
    *done = true;
  }
}

TEST(gmNetwork, RunSync_wait) {

  gmCore::Console::removeAllSinks();
#if 0
  std::shared_ptr<gmCore::OStreamMessageSink> osms =
    std::make_shared<gmCore::OStreamMessageSink>();
  osms->setUseAnsiColor(true);
  osms->setLevel(20);
  osms->initialize();

  std::shared_ptr<gmCore::LogFileMessageSink> lfms =
    std::make_shared<gmCore::LogFileMessageSink>();
  lfms->setLogFilePath("gramods.log");
  lfms->initialize();
#else
  std::shared_ptr<gmCore::NullMessageSink> nullsink =
    std::make_shared<gmCore::NullMessageSink>();
  nullsink->initialize();
#endif

  size_t peer_count = 2;

  std::shared_ptr<std::atomic<bool>> run =
    std::make_shared<std::atomic<bool>>(true);

  std::vector<std::shared_ptr<std::atomic<size_t>>> count_list;
  std::vector<std::shared_ptr<std::atomic<bool>>> done_list;
  std::vector<std::unique_ptr<std::thread>> thread_list;

  for (size_t idx = 0; idx < peer_count; ++idx) {

    std::shared_ptr<std::atomic<size_t>> count = std::make_shared<std::atomic<size_t>>(0);
    std::shared_ptr<std::atomic<bool>> done = std::make_shared<std::atomic<bool>>(false);

    std::unique_ptr<std::thread> thread =
      std::make_unique<std::thread>([idx, peer_count,
                                     count, run, done](){
                                      run_node(idx, peer_count, count, run, done);
                                    });

    count_list.push_back(count);
    done_list.push_back(done);
    thread_list.push_back(std::move(thread));
  }

  size_t loops = 0;
  while (++loops < 100 && *count_list.back() < 500)
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

  GM_DBG2("gTest", "Stopping threads after " << (100*loops) << " ms, " << (*count_list[0]) << " iterations");
  *run = false;

  std::this_thread::sleep_for(std::chrono::milliseconds(2000));

  for (size_t idx = 0; idx < peer_count; ++idx)
    EXPECT_TRUE(*done_list[idx])
      << "Synchronized execution " << idx << " finished";

  for (size_t idx = 1; idx < peer_count; ++idx) {
    EXPECT_LE(*count_list[0] > *count_list[idx] ?
              *count_list[0] - *count_list[idx] :
              *count_list[idx] - *count_list[0], 1)
      << "Loop counts for node 0 and " << idx << " differ by more than one";
  }

  for (auto &thread : thread_list)
    thread->detach();
}

namespace {
  void run_node_2(size_t idx, size_t peer_count) {

    std::shared_ptr<gmNetwork::SyncNode> node =
      std::make_shared<gmNetwork::SyncNode>();

    for (size_t port = PORT1; port < PORT1 + peer_count; ++port) {
      std::stringstream ss;
      ss << "127.0.0.1:" << port;
      node->addPeer(ss.str());
    }
    node->setLocalPeerIdx(idx);
    node->setTimeoutDelay(0.5);
    node->initialize();

    gmNetwork::RunSync * run_sync =
      node->getProtocol<gmNetwork::RunSync>();
    ASSERT_TRUE(run_sync);

    node->waitForConnection();

    std::this_thread::sleep_for(std::chrono::seconds(2));
  }
}

TEST(gmNetwork, SyncNode_pingpong) {

  gmCore::Console::removeAllSinks();
#if 0
  std::shared_ptr<gmCore::OStreamMessageSink> osms =
    std::make_shared<gmCore::OStreamMessageSink>();
  osms->setUseAnsiColor(true);
  osms->setLevel(3);
  osms->initialize();
#else
  std::shared_ptr<gmCore::NullMessageSink> nullsink =
    std::make_shared<gmCore::NullMessageSink>();
  nullsink->initialize();
#endif

  std::stringstream ss;

  std::shared_ptr<gmCore::OStreamMessageSink> ssms =
    std::make_shared<gmCore::OStreamMessageSink>();
  ssms->setStream(&ss);
  ssms->setLevel(4);
  ssms->initialize();

  size_t peer_count = 2;

  std::vector<std::unique_ptr<std::thread>> thread_list;

  for (size_t idx = 0; idx < peer_count; ++idx) {
    std::unique_ptr<std::thread> thread =
      std::make_unique<std::thread>([idx, peer_count](){
                                      run_node_2(idx, peer_count);
                                    });
    thread_list.push_back(std::move(thread));
  }

  std::this_thread::sleep_for(std::chrono::seconds(5));

  for (auto &thread : thread_list)
    thread->detach();

  EXPECT_TRUE(ss.str().find(" 0 Sending ping to 1") != std::string::npos);
  EXPECT_TRUE(ss.str().find(" 1 Sending pong to 0") != std::string::npos);
  EXPECT_TRUE(ss.str().find(" 0 Sending ping to 1") != std::string::npos);
  EXPECT_TRUE(ss.str().find(" 1 Sending pong to 0") != std::string::npos);
}
