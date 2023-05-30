
#include "MyApp.hh"

#include <gmCore/TimeTools.hh>
#include <gmCore/FileResolver.hh>

#include <gmTrack/ButtonsMapper.hh>

#include <gmNetwork/RunSync.hh>
#include <gmNetwork/DataSync.hh>
#include <gmNetwork/SyncSData.hh>
#include <gmNetwork/SyncMData.hh>

#include <osgViewer/Viewer>
#include <osgDB/ReadFile>
#include <osg/MatrixTransform>
#include <osg/LineWidth>
#include <osg/Material>

#include <osg/ComputeBoundsVisitor>

#include <filesystem>

using namespace gramods;

typedef gmNetwork::SyncSData<Eigen::Vector3f> SyncSVec;
typedef gmNetwork::SyncSData<Eigen::Quaternionf> SyncSQuat;

/**
 * Definition of the internal code of MyApp
 */
struct MyApp::Impl {
  Impl(std::vector<std::shared_ptr<gmNetwork::SyncNode>> sync_nodes,
       std::vector<std::shared_ptr<gmTrack::Controller>> controllers,
       std::shared_ptr<gmTrack::SinglePoseTracker> head);

  void setup_sync(std::vector<std::shared_ptr<gmNetwork::SyncNode>> sync_nodes);

  void setup_wand(std::vector<std::shared_ptr<gmTrack::Controller>> controllers);

  void update(gmCore::Updateable::clock::time_point time);

  void update_data(gmCore::Updateable::clock::time_point time);

  void update_states(gmCore::Updateable::clock::time_point time);

  std::shared_ptr<gmGraphics::OsgRenderer> getRenderer();

  // Cluster synchronization handler
  std::shared_ptr<gmNetwork::SyncNode> sync_node;
  bool is_primary = true;

  // Wand (if this exists in the configuration)
  std::shared_ptr<gmTrack::Controller> wand;

  // Head (if this exists in the configuration)
  std::shared_ptr<gmTrack::SinglePoseTracker> head;

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

  // wand analogs
  std::shared_ptr<gmNetwork::SyncMFloat32> sync_analogs =
      std::make_shared<gmNetwork::SyncMFloat32>();

  // wand main button
  std::shared_ptr<gmNetwork::SyncSBool> sync_main_button =
      std::make_shared<gmNetwork::SyncSBool>();

  // wand second button
  std::shared_ptr<gmNetwork::SyncSBool> sync_second_button =
      std::make_shared<gmNetwork::SyncSBool>();

  // wand menu button
  std::shared_ptr<gmNetwork::SyncSBool> sync_menu_button =
      std::make_shared<gmNetwork::SyncSBool>();

  // wand pose (position + orientation)
  std::shared_ptr<SyncSVec> sync_wand_position = std::make_shared<SyncSVec>();
  std::shared_ptr<SyncSQuat> sync_wand_orientation =
      std::make_shared<SyncSQuat>(Eigen::Quaternionf::Identity());

  // head pose (position + orientation)
  std::shared_ptr<SyncSVec> sync_head_position = std::make_shared<SyncSVec>();
  std::shared_ptr<SyncSQuat> sync_head_orientation =
      std::make_shared<SyncSQuat>(Eigen::Quaternionf::Identity());

  /// ----- OSG Stuff -----

  void initOSG();

  osg::ref_ptr<osg::Node> createWand();

  osg::ref_ptr<osg::Material> wand_material = new osg::Material;

  osg::ref_ptr<osg::Group> scenegraph_root = new osg::Group;
  osg::ref_ptr<osg::MatrixTransform> wand_transform = new osg::MatrixTransform;
};


/// ----- External interface implementation -----

MyApp::MyApp(std::vector<std::shared_ptr<gmNetwork::SyncNode>> sync_nodes,
             std::vector<std::shared_ptr<gmTrack::Controller>> controllers,
             std::shared_ptr<gmTrack::SinglePoseTracker> head)
  : _impl(std::make_unique<Impl>(sync_nodes, controllers, head)) {}

MyApp::~MyApp() {}

void MyApp::update(clock::time_point time, size_t frame) {
  _impl->update(time);
}

std::shared_ptr<gmGraphics::OsgRenderer> MyApp::getRenderer() {
  return _impl->getRenderer();
}


/// ----- Internal implementation -----


MyApp::Impl::Impl(
    std::vector<std::shared_ptr<gmNetwork::SyncNode>> sync_nodes,
    std::vector<std::shared_ptr<gmTrack::Controller>> controllers,
    std::shared_ptr<gmTrack::SinglePoseTracker> head)
  : head(head) {

  setup_sync(sync_nodes);
  setup_wand(controllers);
  
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

  if (sync_node->getLocalPeerIdx() != 0) is_primary = false;

  // It is good practice to limit the use of raw pointers to the scope
  // in which you got it, however gmNetwork will keep the raw pointers
  // valid until sync_node is destroyed.
  gmNetwork::DataSync *data_sync =
      sync_node->getProtocol<gmNetwork::DataSync>();

  // Do not forget to add all containers to the synchronization queue
  data_sync->addData(sync_time);
  data_sync->addData(sync_frame_number);
  data_sync->addData(sync_analogs);
  data_sync->addData(sync_main_button);
  data_sync->addData(sync_second_button);
  data_sync->addData(sync_menu_button);
  data_sync->addData(sync_head_position);
  data_sync->addData(sync_head_orientation);
  data_sync->addData(sync_wand_position);
  data_sync->addData(sync_wand_orientation);
}

void MyApp::Impl::setup_wand(
    std::vector<std::shared_ptr<gmTrack::Controller>> controllers) {

  if (controllers.empty())
    // With no wand we are done setting up wands
    return;

  // Only the primary node should handle wand (the replica get the
  // data via network synchronization) but we keep a reference anyways
  // just so that we know to expect wand data.
  // if (!is_primary) return;

  // We could save away more than one wand, but one is enough for now
  wand = controllers[0];
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

  if (head) {
    gmTrack::PoseTracker::PoseSample pose;
    if (head->getPose(pose)) {
      *sync_head_position = pose.position;
      *sync_head_orientation = pose.orientation;
    }
  }

  if (!wand)
    // Only wand stuff below this point, so terminate early if we do
    // not have a wand
    return;

  gmTrack::AnalogsTracker::AnalogsSample analogs;
  if (wand->getAnalogs(analogs))
    *sync_analogs = analogs.analogs;

  gmTrack::ButtonsTracker::ButtonsSample buttons;
  if (wand->getButtons(buttons)) {

    typedef gmTrack::ButtonsMapper::ButtonIdx ButtonIdx;

    if (buttons.buttons.count(ButtonIdx::MAIN))
      *sync_main_button = buttons.buttons[ButtonIdx::MAIN];
    else
      *sync_main_button = false;

    if (buttons.buttons.count(ButtonIdx::SECONDARY))
      *sync_second_button = buttons.buttons[ButtonIdx::SECONDARY];
    else
      *sync_second_button = false;

    if (buttons.buttons.count(ButtonIdx::MENU))
      *sync_menu_button = buttons.buttons[ButtonIdx::MENU];
    else
      *sync_menu_button = false;
  }

  gmTrack::PoseTracker::PoseSample pose;
  if (wand->getPose(pose)) {
    *sync_wand_position = pose.position;
    *sync_wand_orientation = pose.orientation;
  }
}

std::shared_ptr<gmGraphics::OsgRenderer> MyApp::Impl::getRenderer() {
  return osg_renderer;
}

void MyApp::Impl::update_states(gmCore::Updateable::clock::time_point) {

  if (!wand_transform)
    // Cannot update wand transform since we have no wand transform
    return;

  Eigen::Vector3f eP = *sync_wand_position;
  Eigen::Quaternionf eQ = *sync_wand_orientation;

  osg::Vec3 oP(eP.x(), eP.y(), eP.z());
  osg::Quat oQ(eQ.x(), eQ.y(), eQ.z(), eQ.w());

  // Order because OSG uses row vectors: first rotate, then translate
  wand_transform->setMatrix(osg::Matrix::rotate(oQ) *
                            osg::Matrix::translate(oP));

  double R = 0.4, G = 0.4, B = 0.4;
  if (*sync_main_button) R = 0.8;
  if (*sync_second_button) G = 0.8;
  if (*sync_menu_button) B = 0.8;
  wand_material->setEmission(osg::Material::FRONT_AND_BACK, osg::Vec4(R, G, B, 1.0));
}

void MyApp::Impl::initOSG() {

  osg_renderer->setSceneData(scenegraph_root);

  if (wand) {
    // We just have to assume that if a replica should render a wand,
    // because wand data come from the primary, then also the replica
    // will have a wand defined in their config, even if its data are
    // not used. How can we otherwise know if we should render a wand
    // or not?
    osg::ref_ptr<osg::Node> wand_node = createWand();
    wand_transform->addChild(wand_node);
    scenegraph_root->addChild(wand_transform);
  }

  std::string url = "urn:gramods:resources/box.osgt";
  std::filesystem::path path = gmCore::FileResolver::getDefault()->resolve(url);
  osg::ref_ptr<osg::Node> model = osgDB::readNodeFile(path.generic_u8string());

  if (!model.valid()) {
    if (url == path)
      GM_ERR("MyApp", "Could not load file \"" << url << "\"");
    else
      GM_ERR("MyApp", "Could not load file " << path.generic_u8string() << " (" << url << ")");
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

osg::ref_ptr<osg::Node> MyApp::Impl::createWand() {

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

  return geometry;
}
