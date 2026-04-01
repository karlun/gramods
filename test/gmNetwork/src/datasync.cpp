
#include <gmCore/Console.hh>
#include <gmCore/LogFileMessageSink.hh>
#include <gmCore/OStreamMessageSink.hh>
#include <gmCore/Pose.hh>

#include <gmNetwork/SyncNode.hh>
#include <gmNetwork/RunSync.hh>
#include <gmNetwork/DataSync.hh>
#include <gmNetwork/SyncSData.hh>
#include <gmNetwork/SyncJData.hh>
#include <gmNetwork/SyncMData.hh>

#include <atomic>
#include <memory>
#include <thread>
#include <sstream>
#include <random>

using namespace gramods;

#ifdef gramods_ENABLE_nlohmann_json
typedef gmNetwork::SyncJData<float> SyncJFloat;
typedef std::unordered_map<std::string, gmCore::Pose> PoseMap;
typedef gmNetwork::SyncJData<PoseMap> SyncJPose;

void to_json(nlohmann::json &j, const PoseMap &o) {}
void from_json(const nlohmann::json &j, PoseMap &o) {}
#endif

#define PORT2 27040

#define VAL_AS_BOOL false
#define VAL_AS_INT32 28733
#define VAL_AS_FLOAT32 3.14159f
#define VAL_AM_BOOL { false, true }
#define VAL_AM_INT32 { 11234, 64243 }
#define VAL_AM_FLOAT32 { 1.2f, 2.3f }
#define VAL_AJ_FLOAT32 46.4f
#define VAL_AJ_POSE PoseMap {{"a", {.position = Eigen::Vector3f::UnitZ()}}, {"b",{.position = Eigen::Vector3f::UnitY()}}}

#define VAL_BS_BOOL true
#define VAL_BS_INT32 9922123
#define VAL_BS_FLOAT32 0.707f
#define VAL_BM_BOOL { true, false }
#define VAL_BM_INT32 { 2234, 921 }
#define VAL_BM_FLOAT32 { 1e6f, 4.4f }
#define VAL_BJ_FLOAT32 -47.2f
#define VAL_BJ_POSE PoseMap {{"c", {.position = Eigen::Vector3f::UnitX()}}, {"d",{.position = -Eigen::Vector3f::UnitZ()}}}

#define EXPECT_EQ_EIGEN_VECTOR(A, B) EXPECT_GE(1e-10, ((A) - (B)).squaredNorm())

namespace {

  void run_node(size_t idx, size_t peer_count,
                std::shared_ptr<std::atomic<bool>> done) {

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

    std::shared_ptr<gmNetwork::SyncSBool> data_bool(
        std::make_shared<gmNetwork::SyncSBool>(VAL_AS_BOOL));
    std::shared_ptr<gmNetwork::SyncSInt32> data_int32(
        std::make_shared<gmNetwork::SyncSInt32>(VAL_AS_INT32));
    std::shared_ptr<gmNetwork::SyncSFloat32> data_float32(
        std::make_shared<gmNetwork::SyncSFloat32>(VAL_AS_FLOAT32));

#ifdef gramods_ENABLE_nlohmann_json
    std::shared_ptr<SyncJFloat> data_jfloat32(
        std::make_shared<SyncJFloat>(VAL_AJ_FLOAT32));
    std::shared_ptr<SyncJPose> data_jpose_map( //
        std::make_shared<SyncJPose>(VAL_AJ_POSE));
#endif

    gmNetwork::DataSync * data_sync =
      node->getProtocol<gmNetwork::DataSync>();
    data_sync->addData(data_bool);
    data_sync->addData(data_int32);
    data_sync->addData(data_float32);
#ifdef gramods_ENABLE_nlohmann_json
    data_sync->addData(data_jfloat32);
    data_sync->addData(data_jpose_map);
#endif

    GM_DBG1("gTest", "Node " << idx << " waiting for connection");
    node->waitForConnection();

    EXPECT_EQ(*data_bool, VAL_AS_BOOL);
    EXPECT_EQ(*data_int32, VAL_AS_INT32);
    EXPECT_NEAR(*data_float32, VAL_AS_FLOAT32, 1e-6);

#ifdef gramods_ENABLE_nlohmann_json
    EXPECT_NEAR(*data_jfloat32, VAL_AJ_FLOAT32, 1e-6);
    auto pose_map1 = **data_jpose_map;
    EXPECT_TRUE(pose_map1.contains("a"));
    EXPECT_TRUE(pose_map1.contains("b"));
    EXPECT_FALSE(pose_map1.contains("c"));
    EXPECT_FALSE(pose_map1.contains("d"));
#endif

    run_sync->wait();

    if (idx == 0) {
      *data_bool = VAL_BS_BOOL;
      *data_int32 = VAL_BS_INT32;
      *data_float32 = VAL_BS_FLOAT32;
#ifdef gramods_ENABLE_nlohmann_json
      *data_jfloat32 = VAL_BJ_FLOAT32;
      *data_jpose_map = VAL_BJ_POSE;
#endif
    }

    run_sync->wait();

    EXPECT_EQ(*data_bool, VAL_AS_BOOL) << " @ node " << idx;
    EXPECT_EQ(*data_int32, VAL_AS_INT32) << " @ node " << idx;
    EXPECT_NEAR(*data_float32, VAL_AS_FLOAT32, 1e-6) << " @ node " << idx;

#ifdef gramods_ENABLE_nlohmann_json
    EXPECT_NEAR(*data_jfloat32, VAL_AJ_FLOAT32, 1e-6);
    auto pose_map2 = **data_jpose_map;
    EXPECT_TRUE(pose_map2.contains("a"));
    EXPECT_TRUE(pose_map2.contains("b"));
    EXPECT_FALSE(pose_map2.contains("c"));
    EXPECT_FALSE(pose_map2.contains("d"));
    if (pose_map2.contains("b"))
      EXPECT_EQ_EIGEN_VECTOR(pose_map2["b"].position, Eigen::Vector3f::UnitY())
          << " @ node " << idx;
#endif

    run_sync->wait();
    data_sync->update();

    EXPECT_EQ(*data_bool, VAL_BS_BOOL) << " @ node " << idx;
    EXPECT_EQ(*data_int32, VAL_BS_INT32) << " @ node " << idx;
    EXPECT_NEAR(*data_float32, VAL_BS_FLOAT32, 1e-6) << " @ node " << idx;

#ifdef gramods_ENABLE_nlohmann_json
    EXPECT_NEAR(*data_jfloat32, VAL_BJ_FLOAT32, 1e-6);
    auto pose_map3 = **data_jpose_map;
    EXPECT_FALSE(pose_map3.contains("a"));
    EXPECT_FALSE(pose_map3.contains("b"));
    EXPECT_TRUE(pose_map3.contains("c"));
    EXPECT_TRUE(pose_map3.contains("d"));
    if (pose_map2.contains("d"))
      EXPECT_EQ_EIGEN_VECTOR(pose_map2["d"].position, -Eigen::Vector3f::UnitZ())
          << " @ node " << idx;
#endif

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

#if 0
  // Simple type
  auto test1 = std::make_shared<gmNetwork::SyncJData<float>>(0.5);
  *test1 = 1.0;
  std::cerr << *test1 << std::endl;

  // Complex structure with simple types
  auto test2 = std::make_shared<
      gmNetwork::SyncJData<std::map<std::string, std::string>>>(
      std::map<std::string, std::string> {{"a", "b"}, {"b", "a"}});
  std::cerr << (**test2).at("a") << std::endl;

  // Non-simple type
  auto test3 = std::make_shared<gmNetwork::SyncJData<gmCore::Pose>>(
      gmCore::Pose {.position = Eigen::Vector3f::UnitZ()});
  gmCore::Pose mypose = *test3;
  *test3 = mypose;

  // Complex structure with non-simple types
  auto test4 = std::make_shared<
      gmNetwork::SyncJData<std::map<std::string, gmCore::Pose>>>(
      std::map<std::string, gmCore::Pose> {
          {"a", {.position = Eigen::Vector3f::UnitX()}},
          {"b", {.position = Eigen::Vector3f::UnitY()}}});
  std::cerr << (**test4).at("a") << std::endl;
#endif

  size_t peer_count = 2;

  std::vector<std::shared_ptr<std::atomic<bool>>> done_list;
  std::vector<std::unique_ptr<std::thread>> thread_list;

  for (size_t idx = 0; idx < peer_count; ++idx) {

    std::shared_ptr<std::atomic<size_t>> count = std::make_shared<std::atomic<size_t>>(0);
    std::shared_ptr<std::atomic<bool>> done = std::make_shared<std::atomic<bool>>(false);

    std::unique_ptr<std::thread> thread = std::make_unique<std::thread>(
        [idx, peer_count, done]() { run_node(idx, peer_count, done); });

    done_list.push_back(done);
    thread_list.push_back(std::move(thread));
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

