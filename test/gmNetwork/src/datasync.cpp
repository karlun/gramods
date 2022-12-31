
#include <gmNetwork/SyncNode.hh>
#include <gmNetwork/RunSync.hh>
#include <gmNetwork/DataSync.hh>
#include <gmNetwork/SyncSData.hh>
#include <gmNetwork/SyncMData.hh>

#include <gmCore/Console.hh>
#include <gmCore/OStreamMessageSink.hh>
#include <gmCore/LogFileMessageSink.hh>

#include <atomic>
#include <memory>
#include <thread>
#include <sstream>
#include <random>

using namespace gramods;

#define PORT2 27040

#define VAL_AS_BOOL false
#define VAL_AS_INT32 28733
#define VAL_AS_FLOAT32 3.14159f
#define VAL_AM_BOOL { false, true }
#define VAL_AM_INT32 { 11234, 64243 }
#define VAL_AM_FLOAT32 { 1.2f, 2.3f }

#define VAL_BS_BOOL true
#define VAL_BS_INT32 9922123
#define VAL_BS_FLOAT32 0.707f
#define VAL_BM_BOOL { true, false }
#define VAL_BM_INT32 { 2234, 921 }
#define VAL_BM_FLOAT32 { 1e6f, 4.4f }


namespace {

  void run_node(size_t idx, size_t peer_count,
                std::shared_ptr<std::atomic<bool>> done,

                std::shared_ptr<gmNetwork::SyncSBool> data_bool,
                std::shared_ptr<gmNetwork::SyncSInt32> data_int32,
                std::shared_ptr<gmNetwork::SyncSFloat32> data_float32,

                std::shared_ptr<gmNetwork::SyncMBool> data_bools,
                std::shared_ptr<gmNetwork::SyncMInt32> data_int32s,
                std::shared_ptr<gmNetwork::SyncMFloat32> data_float32s) {

    std::shared_ptr<gmNetwork::SyncNode> node =
      std::make_shared<gmNetwork::SyncNode>();

    for (size_t port = PORT2; port < PORT2 + peer_count; ++port) {
      std::stringstream ss;
      ss << "127.0.0.1:" << port;
      node->addPeer(ss.str());
    }
    node->setLocalPeerIdx(idx);
    node->initialize();

    gmNetwork::RunSync * run_sync =
      node->getProtocol<gmNetwork::RunSync>();
    ASSERT_TRUE(run_sync);

    gmNetwork::DataSync * data_sync =
      node->getProtocol<gmNetwork::DataSync>();
    data_sync->addData(data_bool);
    data_sync->addData(data_int32);
    data_sync->addData(data_float32);

    GM_DBG1("gTest", "Node " << idx << " waiting for connection");
    node->waitForConnection();

    EXPECT_EQ(*data_bool, VAL_AS_BOOL);
    EXPECT_EQ(*data_int32, VAL_AS_INT32);
    EXPECT_NEAR(*data_float32, VAL_AS_FLOAT32, 1e-6);

    run_sync->wait();

    if (idx == 0) {
      *data_bool = VAL_BS_BOOL;
      *data_int32 = VAL_BS_INT32;
      *data_float32 = VAL_BS_FLOAT32;
    }

    run_sync->wait();

    EXPECT_EQ(*data_bool, VAL_AS_BOOL) << " @ node " << idx;
    EXPECT_EQ(*data_int32, VAL_AS_INT32) << " @ node " << idx;
    EXPECT_NEAR(*data_float32, VAL_AS_FLOAT32, 1e-6) << " @ node " << idx;

    run_sync->wait();
    data_sync->update();

    EXPECT_EQ(*data_bool, VAL_BS_BOOL) << " @ node " << idx;
    EXPECT_EQ(*data_int32, VAL_BS_INT32) << " @ node " << idx;
    EXPECT_NEAR(*data_float32, VAL_BS_FLOAT32, 1e-6) << " @ node " << idx;
   
    *done = true;
  }
}

TEST(gmNetwork, DataSync_singles) {

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

  size_t peer_count = 10;

  std::vector<std::shared_ptr<std::atomic<bool>>> done_list;
  std::vector<std::unique_ptr<std::thread>> thread_list;

  // "Shared" data
  std::vector<std::shared_ptr<gmNetwork::SyncSBool>> data_bool_list;
  std::vector<std::shared_ptr<gmNetwork::SyncSInt32>> data_int32_list;
  std::vector<std::shared_ptr<gmNetwork::SyncSFloat32>> data_float32_list;

  std::vector<std::shared_ptr<gmNetwork::SyncMBool>> data_bools_list;
  std::vector<std::shared_ptr<gmNetwork::SyncMInt32>> data_int32s_list;
  std::vector<std::shared_ptr<gmNetwork::SyncMFloat32>> data_float32s_list;

  for (size_t idx = 0; idx < peer_count; ++idx) {

    std::shared_ptr<std::atomic<size_t>> count = std::make_shared<std::atomic<size_t>>(0);
    std::shared_ptr<std::atomic<bool>> done = std::make_shared<std::atomic<bool>>(false);

    std::shared_ptr<gmNetwork::SyncSBool>
      val_sB(std::make_shared<gmNetwork::SyncSBool>(VAL_AS_BOOL));
    std::shared_ptr<gmNetwork::SyncSInt32>
      val_sI(std::make_shared<gmNetwork::SyncSInt32>(VAL_AS_INT32));
    std::shared_ptr<gmNetwork::SyncSFloat32>
      val_sF(std::make_shared<gmNetwork::SyncSFloat32>(VAL_AS_FLOAT32));

    std::shared_ptr<gmNetwork::SyncMBool>
      val_mB(std::make_shared<gmNetwork::SyncMBool>());
    std::shared_ptr<gmNetwork::SyncMInt32>
      val_mI(std::make_shared<gmNetwork::SyncMInt32>());
    std::shared_ptr<gmNetwork::SyncMFloat32>
      val_mF(std::make_shared<gmNetwork::SyncMFloat32>());

    *val_mB = VAL_AM_BOOL;
    *val_mI = VAL_AM_INT32;
    *val_mF = VAL_AM_FLOAT32;

    std::unique_ptr<std::thread> thread =
      std::make_unique<std::thread>([idx, peer_count, done,
                                     val_sB, val_sI, val_sF,
                                     val_mB, val_mI, val_mF](){
                                      run_node(idx, peer_count, done,
                                               val_sB, val_sI, val_sF,
                                               val_mB, val_mI, val_mF);
                                               });

    done_list.push_back(done);
    thread_list.push_back(std::move(thread));

    data_bool_list.push_back(val_sB);
    data_int32_list.push_back(val_sI);
    data_float32_list.push_back(val_sF);

    data_bools_list.push_back(val_mB);
    data_int32s_list.push_back(val_mI);
    data_float32s_list.push_back(val_mF);
  }

  for (int idx = 0; idx < 10000; ++idx) {

    bool all_done = true;

    for (auto done : done_list)
      if (!*done) all_done = false;

    if (all_done) break;

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }

  for (size_t idx = 0; idx < peer_count; ++idx)
    EXPECT_TRUE(*done_list[idx])
      << " @ node " << idx;

  for (auto &thread : thread_list)
    thread->detach();
}

