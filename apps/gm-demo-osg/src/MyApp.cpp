
#include <gmCore/Pose.hh>
#include <gmTrack/TrackerBase.hh>
#include <gmTrack/StdKey.hh>

#include "MyApp.hh"

#include <gmCore/TimeTools.hh>
#include <gmCore/FileResolver.hh>

#include <gmNetwork/RunSync.hh>
#include <gmNetwork/DataSync.hh>
#include <gmNetwork/SyncSData.hh>
#include <gmNetwork/SyncJData.hh>

#include <osgViewer/Viewer>
#include <osgDB/ReadFile>
#include <osg/MatrixTransform>
#include <osg/LineWidth>
#include <osg/Material>

#include <osg/ComputeBoundsVisitor>

#include <filesystem>

using namespace gramods;

typedef gmNetwork::SyncJData<gmTrack::PoseTracker::State> SyncJPose;
typedef gmNetwork::SyncJData<gmTrack::BinaryTracker::State> SyncJBinary;
typedef gmNetwork::SyncJData<gmTrack::FloatTracker::State> SyncJFloat;
typedef gmNetwork::SyncJData<gmTrack::Float2Tracker::State> SyncJFloat2;

/**
 * Definition of the internal code of MyApp
 */
struct MyApp::Impl {
  Impl(std::vector<std::shared_ptr<gmNetwork::SyncNode>> sync_nodes,
       std::shared_ptr<gmTrack::TrackerSet> trackers);

  void setup_sync(std::vector<std::shared_ptr<gmNetwork::SyncNode>> sync_nodes);

  void update(gmCore::Updateable::clock::time_point time);

  void update_data(gmCore::Updateable::clock::time_point time);

  void update_states(gmCore::Updateable::clock::time_point time);

  std::shared_ptr<gmGraphics::OsgRenderer> getRenderer();

  // Cluster synchronization handler
  std::shared_ptr<gmNetwork::SyncNode> sync_node;
  bool is_primary = true;

  // Tracker data, if they exist
  std::shared_ptr<gmTrack::TrackerSet> trackers;

  // The renderer calls OpenSceneGraph for us
  std::shared_ptr<gmGraphics::OsgRenderer> osg_renderer =
      std::make_shared<gmGraphics::OsgRenderer>();

  /// ----- Containers for synchronized data -----

  // steady time
  std::shared_ptr<gmNetwork::SyncSFloat64> sync_time =
      std::make_shared<gmNetwork::SyncSFloat64>();

  // steady frame number
  std::shared_ptr<gmNetwork::SyncSUInt64> sync_frame_number =
      std::make_shared<gmNetwork::SyncSUInt64>(0);

  // Pose tracker data
  std::shared_ptr<SyncJPose> sync_pose = std::make_shared<SyncJPose>();

  // Binary tracker data
  std::shared_ptr<SyncJBinary> sync_binary = std::make_shared<SyncJBinary>();

  // Float tracker data
  std::shared_ptr<SyncJFloat> sync_float = std::make_shared<SyncJFloat>();

  // Float2 tracker data
  std::shared_ptr<SyncJFloat2> sync_float2 = std::make_shared<SyncJFloat2>();

  /// ----- OSG Stuff -----

  void initOSG();
  void initWand();

  osg::ref_ptr<osg::Group> scenegraph_root = new osg::Group;

  osg::ref_ptr<osg::MatrixTransform> wand_transform = nullptr;
  osg::ref_ptr<osg::Material> wand_material = new osg::Material;
};


/// ----- External interface implementation -----

MyApp::MyApp(std::vector<std::shared_ptr<gmNetwork::SyncNode>> sync_nodes,
             std::shared_ptr<gmTrack::TrackerSet> trackers)
  : _impl(std::make_unique<Impl>(sync_nodes, trackers)) {}

MyApp::~MyApp() {}

void MyApp::update(clock::time_point time, size_t frame) {
  _impl->update(time);
}

std::shared_ptr<gmGraphics::OsgRenderer> MyApp::getRenderer() {
  return _impl->getRenderer();
}


/// ----- Internal implementation -----

MyApp::Impl::Impl(std::vector<std::shared_ptr<gmNetwork::SyncNode>> sync_nodes,
                  std::shared_ptr<gmTrack::TrackerSet> trackers)
  : trackers(trackers) {

  setup_sync(sync_nodes);
  
  initOSG();
}

void MyApp::Impl::setup_sync(
    std::vector<std::shared_ptr<gmNetwork::SyncNode>> sync_nodes) {

  if (!sync_nodes.empty())
    // There should be only one SyncNode, but any excess will be
    // ignored anyways.
    sync_node = sync_nodes[0];
  else {
    // The config did not provide a SyncNode, so create one! We need
    // this so that we can use the sync_* variables even on a single
    // node without peers.
    sync_node = std::make_shared<gmNetwork::SyncNode>();
    sync_node->setLocalPeerIdx(0);
    sync_node->addPeer(""); //< Anything here since our own peer will
                            //  be ignored
    sync_node->initialize();
  }

  is_primary = (sync_node->getLocalPeerIdx() == 0);

  // It is good practice to limit the use of raw pointers to the scope
  // in which you got it, however gmNetwork will keep the raw pointers
  // valid until sync_node is destroyed.
  gmNetwork::DataSync *data_sync =
      sync_node->getProtocol<gmNetwork::DataSync>();

  // Do not forget to add all containers to the synchronization queue
  data_sync->addData(sync_time);
  data_sync->addData(sync_frame_number);
  data_sync->addData(sync_pose);
  data_sync->addData(sync_binary);
  data_sync->addData(sync_float);
  data_sync->addData(sync_float2);
}

void MyApp::Impl::update(gmCore::Updateable::clock::time_point time) {

  // Wait until we are connected to all peers before starting to
  // update data, animate and stuff
  if (!sync_node->isConnected())
    return;

  gmNetwork::DataSync *data_sync =
      sync_node->getProtocol<gmNetwork::DataSync>();
  gmNetwork::RunSync *run_sync =
      sync_node->getProtocol<gmNetwork::RunSync>();

  update_data(time);   // Let the primary update internal data

  run_sync->wait();    // Wait for primary to have sent its values
  data_sync->update(); // Swap old values for newly synchronized

  update_states(time); // Use the data to update scenegraph states
}

void MyApp::Impl::update_data(gmCore::Updateable::clock::time_point time) {

  if (!is_primary)
    // Only primary update internal states, the rest wait for incoming
    // data via the DataSync instance.
    return;

  // Setting data to a SyncData instance (that has been added to a
  // DataSync instance) will send this value to all other nodes and
  // end up in the corresponding instance's back buffer.

  *sync_time = gmCore::TimeTools::timePointToSeconds(time);
  *sync_frame_number = *sync_frame_number + 1;

  // Read off and synchronize all tracker data
  if (auto s = trackers->getPose()) *sync_pose = *s;
  if (auto s = trackers->getBinary()) *sync_binary = *s;
  if (auto s = trackers->getFloat()) *sync_float = *s;
  if (auto s = trackers->getFloat2()) *sync_float2 = *s;
}

std::shared_ptr<gmGraphics::OsgRenderer> MyApp::Impl::getRenderer() {
  return osg_renderer;
}

void MyApp::Impl::update_states(gmCore::Updateable::clock::time_point) {

  const auto pose_states = **sync_pose;
  const auto binary_states = **sync_binary;
  const auto float_states = **sync_float;
  const auto float2_states = **sync_float2;

  if (!wand_transform &&
      pose_states.contains(gmTrack::StdKey::PRIMARY_WAND))
      [[unlikely]]
    initWand();

  if (!wand_transform) [[unlikely]]
    // Cannot update wand without wand transform or data
    return;

  gmCore::Pose wand_pose =
      pose_states.at(gmTrack::StdKey::PRIMARY_WAND).value;
  Eigen::Vector3f eP = wand_pose.position;
  Eigen::Quaternionf eQ = wand_pose.orientation;

  osg::Vec3 oP(eP.x(), eP.y(), eP.z());
  osg::Quat oQ(eQ.x(), eQ.y(), eQ.z(), eQ.w());

  // Order because OSG uses row vectors: first rotate, then translate
  wand_transform->setMatrix(osg::Matrix::rotate(oQ) *
                            osg::Matrix::translate(oP));

  double R = 0.4, G = 0.4, B = 0.4;
  if (binary_states.contains(gmTrack::StdKey::MAIN_BUTTON) &&
      binary_states.at(gmTrack::StdKey::MAIN_BUTTON).value)
    R = 0.8;
  if (binary_states.contains(gmTrack::StdKey::SECOND_BUTTON) &&
      binary_states.at(gmTrack::StdKey::SECOND_BUTTON).value)
    G = 0.8;
  if (binary_states.contains(gmTrack::StdKey::MENU_BUTTON) &&
      binary_states.at(gmTrack::StdKey::MENU_BUTTON).value)
    B = 0.8;
  wand_material->setEmission(osg::Material::FRONT_AND_BACK,
                             osg::Vec4(R, G, B, 1.0));
}

void MyApp::Impl::initOSG() {

  osg_renderer->setSceneData(scenegraph_root);

  std::string url = "urn:gramods:resources/box.osgt";
  std::filesystem::path path = gmCore::FileResolver::getDefault()->resolve(url);
  osg::ref_ptr<osg::Node> model = osgDB::readNodeFile(path.generic_string());

  if (!model.valid()) {
    if (url == path)
      GM_ERR("MyApp", "Could not load file \"" << url << "\"");
    else
      GM_ERR("MyApp", "Could not load file " << path.generic_string() << " (" << url << ")");
    return;
  }

  GM_DBG1("MyApp", "Model loaded successfully!");

  osg::ref_ptr<osg::MatrixTransform> model_transform =
      new osg::MatrixTransform();
  scenegraph_root->addChild(model_transform);
  model_transform->addChild(model);

  //get the bounding box
  osg::ComputeBoundsVisitor cbv;
  osg::BoundingBox &bb(cbv.getBoundingBox());
  model->accept(cbv);

  osg::Vec3f tmpVec = bb.center();
  float tmpScale = 1.f / bb.radius();

  // scale to fit model and translate model center to origin
  model_transform->postMult(osg::Matrix::translate(-tmpVec));
  model_transform->postMult(osg::Matrix::scale(tmpScale, tmpScale, tmpScale));

  GM_DBG1("MyApp",
          "Model bounding sphere center: " << tmpVec[0] << ", " << tmpVec[1]
                                           << ", " << tmpVec[2]);
  GM_DBG1("MyApp", "Model bounding sphere radius: " << tmpScale);

  //disable face culling
  model->getOrCreateStateSet()->setMode(
      GL_CULL_FACE, osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE);
}

void MyApp::Impl::initWand() {

  osg::ref_ptr<osg::Geometry> geometry = new osg::Geometry;

  osg::ref_ptr<osg::LineWidth> lineWidth = new osg::LineWidth();
  lineWidth->setWidth(4);
  geometry->getOrCreateStateSet()->setAttributeAndModes(
      lineWidth, osg::StateAttribute::ON);

  osg::Vec3Array *vertices = new osg::Vec3Array;
  vertices->push_back(osg::Vec3d(0, 0, 0));    // From origin
  vertices->push_back(osg::Vec3d(0, 0, -1000)); //  to 1 km ahead
  geometry->setVertexArray(vertices);

  geometry->addPrimitiveSet(new osg::DrawArrays(GL_LINES, 0, vertices->size()));

  wand_material->setEmission(osg::Material::FRONT_AND_BACK, osg::Vec4(1.0, 1.0, 1.0, 1.0));
  geometry->getOrCreateStateSet()->setAttributeAndModes(
      wand_material, osg::StateAttribute::PROTECTED);

  wand_transform = new osg::MatrixTransform;
  wand_transform->addChild(geometry);
  scenegraph_root->addChild(wand_transform);
}
