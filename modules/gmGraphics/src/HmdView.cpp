
#include <gmGraphics/HmdView.hh>

#ifdef gramods_ENABLE_OpenVR

#include <gmCore/RunOnce.hh>
#include <gmCore/RunLimited.hh>

#include <gmGraphics/OffscreenRenderTargets.hh>
#include <gmGraphics/RasterProcessor.hh>

#include <GL/glew.h>
#include <GL/gl.h>

BEGIN_NAMESPACE_GMGRAPHICS;

GM_OFI_DEFINE_SUB(HmdView, View);
GM_OFI_PARAM2(HmdView, showEye, size_t, setShowEye);
GM_OFI_POINTER2(HmdView, openVR, gmCore::OpenVR, setOpenVR);

struct HmdView::Impl {

  void renderFullPipeline(ViewSettings settings);

  void setup();
  bool is_setup = false;
  bool is_functional = false;

  void processEye(ViewSettings settings,
                  vr::IVRCompositor *compositor,
                  vr::EVREye eye);

  size_t show_eye = 0;
  std::shared_ptr<gmCore::OpenVR> openvr;

  static const std::string fragment_code;
  uint32_t render_width = 1920;
  uint32_t render_height = 1080;

  OffscreenRenderTargets render_target;
  RasterProcessor raster_processor;

  size_t cache_frame_number = std::numeric_limits<size_t>::max();
};

HmdView::HmdView() : _impl(std::make_unique<Impl>()) {}

const std::string HmdView::Impl::fragment_code =
    R"lang=glsl(
#version 330 core

uniform sampler2D tex_left;
uniform sampler2D tex_right;
uniform int show_eye;

in vec2 position;
out vec4 fragColor;

void main() {

  if (show_eye == 0) {
    fragColor = texture(tex_left, position * 0.5 + 0.5);
  } else if (show_eye == 1) {
    fragColor = texture(tex_right, position * 0.5 + 0.5);
  } else {
    if (position.x < 0) {
      fragColor = texture(tex_left, position * vec2(1.0, 0.5) + vec2(1.0, 0.5));
    } else {
      fragColor = texture(tex_right, position * vec2(1.0, 0.5) + vec2(0.0, 0.5));
    }
  }
}
)lang=glsl";

void HmdView::Impl::setup() {
  is_setup = true;

  openvr->ptr()->GetRecommendedRenderTargetSize(&render_width, &render_height);

  render_target.setPixelFormat(GL_RGBA8);
  raster_processor.setFragmentCode(fragment_code);

  if (render_target.init(2) && raster_processor.init()) is_functional = true;
}

void HmdView::Impl::processEye(ViewSettings settings,
                               vr::IVRCompositor *compositor,
                               vr::EVREye eye) {

  float left, right, top, bottom;
  openvr->ptr()->GetProjectionRaw(eye, &left, &right, &bottom, &top);

  Camera camera(settings.frame_number);
  camera.setClipPlanes(left, right, bottom, top);

  auto pose_list = openvr->getPoseList();
  if (!settings.viewpoints.empty()) {
    camera.setPosition(settings.viewpoints[0]->getPosition());
    camera.setOrientation(settings.viewpoints[0]->getOrientation());
    if (settings.viewpoints.size() > 1)
      GM_RUNONCE(GM_WRN("HmdView",
                        "No support for multiple viewpoints (using 1 of "
                            << settings.viewpoints.size() << ")"));
  } else if (pose_list) {
    Eigen::Matrix4f M =
	  gmCore::OpenVR::convert(
        (*pose_list)[vr::k_unTrackedDeviceIndex_Hmd].mDeviceToAbsoluteTracking) *
	  gmCore::OpenVR::convert(openvr->ptr()->GetEyeToHeadTransform(eye));
    Eigen::Quaternionf Q_rot;
    Q_rot = M.block<3, 3>(0, 0);
    camera.setPosition(M.block<3, 1>(0, 3));
    camera.setOrientation(Q_rot);
  }

  render_target.bind(render_width, render_height, eye);

  // TODO: Both eyes should render with the same near/far distances or
  // we might end up with inconsistencies between the eyes
  settings.renderNodes(camera);

  if (!compositor) return;

  vr::Texture_t eyeTexture = {(void *)(uintptr_t)render_target.getTexId(eye),
                              vr::TextureType_OpenGL,
                              vr::ColorSpace_Gamma};
  vr::EVRCompositorError err = compositor->Submit(eye, &eyeTexture);
  if (err != vr::VRCompositorError_None)
    GM_ERR("HmdView", "Could not submit graphics: " << err);
}

void HmdView::renderFullPipeline(ViewSettings settings) {
  populateViewSettings(settings);
  _impl->renderFullPipeline(settings);
}

void HmdView::Impl::renderFullPipeline(ViewSettings settings) {

  if (!openvr) {
    GM_RUNONCE(
        GM_ERR("HmdView", "Cannot render HMD view without OpenVR instance"));
    return;
  }

  if (!is_setup) setup();
  if (!is_functional) return;

  vr::IVRCompositor *compositor = vr::VRCompositor();
  if (!compositor) {
    GM_RUNLIMITED(GM_ERR("HmdView", "No compositor available"), 10);
  }

  // Render renderers to offscreen buffers and submit to OpenVR

  if (cache_frame_number != settings.frame_number) {
    settings.pixel_format = GL_RGBA8;

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    render_target.push();
    processEye(settings, compositor, vr::Eye_Left);
    processEye(settings, compositor, vr::Eye_Right);
    render_target.pop();

    glFlush();
  }

  glDisable(GL_DEPTH_TEST);

  // Render offscreen buffers to active render target

  GM_DBG2("SaveView", "Render offscreen buffers to active render target");

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, render_target.getTexId(0));
  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, render_target.getTexId(1));

  GLuint program_id = raster_processor.getProgramId();
  glUseProgram(program_id);
  glUniform1i(glGetUniformLocation(program_id, "tex_left"), 0);
  glUniform1i(glGetUniformLocation(program_id, "tex_right"), 1);
  glUniform1i(glGetUniformLocation(program_id, "show_eye"), show_eye);

  raster_processor.run();

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, 0);
  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, 0);
}

void HmdView::setShowEye(size_t eye) {
  if (eye > 2)
    throw gmCore::InvalidArgument(GM_STR("Cannot show eye " << eye << " > 2"));
  _impl->show_eye = eye;
}

void HmdView::setOpenVR(std::shared_ptr<gmCore::OpenVR> openvr) {
  _impl->openvr = openvr;
}

void HmdView::traverse(Visitor *visitor) {
  if (_impl->openvr) _impl->openvr->accept(visitor);
}

END_NAMESPACE_GMGRAPHICS;

#endif
