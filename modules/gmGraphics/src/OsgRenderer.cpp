
#include <gmGraphics/OsgRenderer.hh>

#ifdef gramods_ENABLE_OpenSceneGraph

#include <osgViewer/Viewer>

BEGIN_NAMESPACE_GMGRAPHICS;

struct OsgRenderer::Impl {

  void setup();

  void render(Camera camera, float near, float far);

  void getNearFar(Camera camera, float &near, float &far);

  void setSceneData(osg::Node *node);

  void update(Updateable::clock::time_point t);

  bool is_initialized = false;
  osg::ref_ptr<osgViewer::Viewer> viewer;
  osg::ref_ptr<osg::Node> tmp_scene_data;
};

OsgRenderer::OsgRenderer() : Updateable(0), _impl(std::make_unique<Impl>()) {}

OsgRenderer::~OsgRenderer() {}

void OsgRenderer::render(Camera camera, float near, float far) {
  _impl->render(camera, near, far);
}

void OsgRenderer::getNearFar(Camera camera, float &near, float &far) {
  _impl->getNearFar(camera, near, far);
}

void OsgRenderer::setSceneData(osg::Node *node) {
  _impl->setSceneData(node);
}

void OsgRenderer::update(clock::time_point t) {
  _impl->update(t);
}

/// ----- Impl -----

void OsgRenderer::Impl::setup() {

  viewer = new osgViewer::Viewer;
  viewer->setThreadingModel(osgViewer::Viewer::SingleThreaded);

  // Set up osgViewer::GraphicsWindowEmbedded for this context
  osg::ref_ptr<::osg::GraphicsContext::Traits> traits =
      new osg::GraphicsContext::Traits;

  osg::ref_ptr<osgViewer::GraphicsWindowEmbedded> graphicsWindow =
      new osgViewer::GraphicsWindowEmbedded(traits.get());

  osg::Camera * osg_cam = viewer->getCamera();
  osg_cam->setGraphicsContext(graphicsWindow.get());

  // disable osg from clearing the buffers that are handled by gramods
  GLbitfield tmpMask = osg_cam->getClearMask();
  osg_cam->setClearMask(tmpMask &
                       (~(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)));

  if (tmp_scene_data) viewer->setSceneData(tmp_scene_data);

  is_initialized = true;
}

void OsgRenderer::Impl::render(Camera camera, float near, float far) {

  if (!is_initialized) setup();

  osg::Camera *osg_cam = viewer->getCamera();

  GLint vp[4];
  glGetIntegerv(GL_VIEWPORT, vp);
  osg_cam->setViewport(vp[0], vp[1], vp[2], vp[3]);

  Eigen::Matrix4f proj = camera.getProjectionMatrix(near, far);
  osg_cam->setProjectionMatrix(osg::Matrix(proj.data()));

  Eigen::Matrix4f view = camera.getViewMatrix().matrix();
  osg_cam->setViewMatrix(osg::Matrix(view.data()));

  viewer->renderingTraversals();
}

void OsgRenderer::Impl::getNearFar(Camera, float &, float &) {
  // We currently ignore near and far distances
}

void OsgRenderer::Impl::setSceneData(osg::Node *node) {
  if (viewer)
    viewer->setSceneData(node);
  else
    tmp_scene_data = node;
}

void OsgRenderer::Impl::update(clock::time_point t) {
  if (!viewer) return;
  viewer->eventTraversal();
  viewer->updateTraversal();
}

END_NAMESPACE_GMGRAPHICS;

#endif
