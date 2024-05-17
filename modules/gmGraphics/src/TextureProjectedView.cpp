
#include <gmGraphics/TextureProjectedView.hh>

#include <gmGraphics/GLUtils.hh>
#include <gmCore/RunOnce.hh>

#include <gmGraphics/OffscreenRenderTargets.hh>
#include <gmGraphics/RasterProcessor.hh>

#include <Eigen/Eigen>

BEGIN_NAMESPACE_GMGRAPHICS;

GM_OFI_DEFINE_SUB(TextureProjectedView, MultiscopicView);
GM_OFI_POINTER2(TextureProjectedView, texture, gmGraphics::TextureInterface, setTexture);
GM_OFI_PARAM2(TextureProjectedView, bufferWidth, gmCore::size2, setBufferResolution);
GM_OFI_PARAM2(TextureProjectedView, linearInterpolation, bool, setLinearInterpolation);
GM_OFI_PARAM2(TextureProjectedView, warpScale, Eigen::Vector3f, setWarpScale);
GM_OFI_PARAM2(TextureProjectedView, warpOffset, Eigen::Vector3f, setWarpOffset);

struct TextureProjectedView::Impl {

  static const std::string fragment_code;

  OffscreenRenderTargets render_target;
  RasterProcessor raster_processor;

  bool is_setup = false;
  bool is_functional = false;

  void renderFullPipeline(ViewSettings settings, Eye eye);
  void renderFullPipeline(ViewSettings settings, Eye eye, Viewpoint *viewpoint);

  gmCore::size2 buffer_res = {2048, 2048};

  std::shared_ptr<TextureInterface> texture;

  std::vector<Eigen::Vector3f> hull_points;

  bool edgeToEdge = false;
  Eigen::Vector3f scale = Eigen::Vector3f::Ones();
  Eigen::Vector3f offset = Eigen::Vector3f::Zero();
};

const std::string TextureProjectedView::Impl::fragment_code = R"lang=glsl(
#version 330 core

uniform mat4 rPV;
uniform sampler2D view_id;
uniform sampler2D warp_id;
uniform bool edge_to_edge;
uniform vec3 scale;
uniform vec3 offset;

in vec2 position;
out vec4 fragColor;

void main() {

  vec2 pos = position * 0.5 + 0.5;

  if (edge_to_edge) {
    ivec2 warp_size = textureSize(warp_id, 0);
    vec2 pix_w = 1 / warp_size;
    pos = pos * (1 - pix_w) + 0.5 * pix_w;
  }

  vec3 warp_coord = texture(warp_id, pos).xyz;
  warp_coord = warp_coord * scale + offset;

  // projection point to render raster coordinate
  vec4 t = rPV * vec4(warp_coord, 1);
  vec2 uv = (t.xy / t.w) * 0.5 + vec2(0.5, 0.5);

  if (uv.x >= 0 && uv.x <= 1 &&
      uv.y >= 0 && uv.y <= 1)
    fragColor = texture(view_id, uv);
  else
    fragColor = vec4(0);
}
)lang=glsl";

TextureProjectedView::TextureProjectedView()
  : _impl(std::make_unique<Impl>()) {}

TextureProjectedView::~TextureProjectedView() {}

void TextureProjectedView::setBufferResolution(gmCore::size2 R) {
  _impl->buffer_res = R;
}

void TextureProjectedView::setLinearInterpolation(bool on) {
  _impl->render_target.setLinearInterpolation(on);
}

void TextureProjectedView::renderFullPipeline(ViewSettings settings, Eye eye) {
  _impl->renderFullPipeline(settings, eye);
}

void TextureProjectedView::Impl::renderFullPipeline(ViewSettings settings,
                                                    Eye eye) {

  if (!texture) {
    GM_RUNONCE(GM_ERR(
        "TextureProjectedView",
        "Missing texture to project rendering against."));
    return;
  }

  for (auto viewpoint : settings.viewpoints)
    renderFullPipeline(settings, eye, viewpoint.get());
}

void TextureProjectedView::Impl::renderFullPipeline(ViewSettings settings,
                                                    Eye eye,
                                                    Viewpoint *viewpoint) {

  Eigen::Vector3f x_VP = viewpoint->getPosition(eye);

  if (hull_points.empty()) {
    // No hull points specified, so we use the hull of the *reach* of
    // the texture, which is 0-1 times scale and plus offset in each
    // dimension.
    GM_DBG1("TextureProjectedView",
            "No hull points specified; using hull of texture reach ("
                << offset.transpose() << " - " << (scale + offset).transpose()
                << ").");
    float x = scale.x(), y = scale.y(), z = scale.z();
    hull_points.push_back(Eigen::Vector3f(0, 0, 0) + offset);
    hull_points.push_back(Eigen::Vector3f(x, 0, 0) + offset);
    hull_points.push_back(Eigen::Vector3f(0, y, 0) + offset);
    hull_points.push_back(Eigen::Vector3f(x, y, 0) + offset);
    hull_points.push_back(Eigen::Vector3f(0, 0, z) + offset);
    hull_points.push_back(Eigen::Vector3f(x, 0, z) + offset);
    hull_points.push_back(Eigen::Vector3f(0, y, z) + offset);
    hull_points.push_back(Eigen::Vector3f(x, y, z) + offset);
  }

  Camera camera(settings);
  bool camera_good = camera.setLookAtPoints(x_VP, hull_points);
  if (!camera_good) {
    GM_RUNONCE(GM_ERR("TextureProjectedView",
                      "Cannot render with bad viewpoint camera."));
    return;
  }

  camera.setEye(eye);

  if (!is_setup) {
    is_setup = true;
    raster_processor.setFragmentCode(fragment_code);
    render_target.setPixelFormat(settings.pixel_format);
    if (render_target.init() && raster_processor.init()) is_functional = true;
  }

  if (!is_functional) {
    GM_RUNONCE(
        GM_ERR("TextureProjectedView", "Dysfunctional internal GL workings."));
    return;
  }

  // Render all renderers to the offscreen buffer
  GM_DBG2("TextureProjectedView",
          "Render all renderers to the offscreen buffer.");

  render_target.push();
  render_target.bind(buffer_res[0], buffer_res[1]);

  glEnable(GL_DEPTH_TEST);

  settings.renderNodes(camera);

  render_target.pop();

  // Render offscreen buffer to active render target
  GM_DBG2("TextureProjectedView",
          "Render offscreen buffer to active render target");

  glDisable(GL_DEPTH_TEST);

  GLuint warp_id = texture->updateTexture(settings.frame_number, Eye::MONO);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, render_target.getTexId());
  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, warp_id);

  GLuint program_id = raster_processor.getProgramId();
  glUseProgram(program_id);

  glUniform1i(glGetUniformLocation(program_id, "view_id"), 0);
  glUniform1i(glGetUniformLocation(program_id, "warp_id"), 1);
  glUniform1i(glGetUniformLocation(program_id, "edge_to_edge"), edgeToEdge);
  glUniform3fv(glGetUniformLocation(program_id, "scale"), 1, scale.data());
  glUniform3fv(glGetUniformLocation(program_id, "offset"), 1, offset.data());

  camera.setNearFar(1, 2);
  Eigen::Matrix4f rPV =
      camera.getProjectionMatrix() * camera.getViewMatrix().matrix();
  glUniformMatrix4fv(
      glGetUniformLocation(program_id, "rPV"), 1, false, rPV.data());

  GM_RUNONCE_BEGIN;
  GM_DBG1("TextureProjectedView", "Hull in projected coordinates");
  for (auto &pt : hull_points) {
    auto x = (rPV * pt.homogeneous()).hnormalized();
    GM_DBG1("TextureProjectedView", "  " << x.transpose());
  }
  GM_RUNONCE_END;

  raster_processor.run();

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, 0);
  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, 0);
}

void TextureProjectedView::setTexture(std::shared_ptr<TextureInterface> tex) {
  _impl->texture = tex;
}

void TextureProjectedView::setEdgeToEdge(bool on) {
  _impl->edgeToEdge = on;
}

void TextureProjectedView::setWarpScale(Eigen::Vector3f scale) {
  _impl->scale = scale;
}

void TextureProjectedView::setWarpOffset(Eigen::Vector3f offset) {
  _impl->offset = offset;
}

void TextureProjectedView::traverse(Visitor *visitor) {
  MultiscopicView::traverse(visitor);
  if (auto obj = std::dynamic_pointer_cast<gmCore::Object>(_impl->texture))
    obj->accept(visitor);
}

END_NAMESPACE_GMGRAPHICS;
