
#include <gmGraphics/NdiView.hh>

#ifdef gramods_ENABLE_NDI

#include <gmCore/FileResolver.hh>
#include <gmCore/ImportLibrary.hh>
#include <gmCore/RunOnce.hh>
#include <gmCore/Stringify.hh>

#include <gmGraphics/OffscreenRenderTargets.hh>
#include <gmGraphics/PixelPacker.hh>
#include <gmGraphics/RasterProcessor.hh>

#include <Processing.NDI.Lib.h>

#include <GL/glew.h>
#include <GL/gl.h>

#include <chrono>
#include <optional>
#include <thread>
#include <condition_variable>


BEGIN_NAMESPACE_GMGRAPHICS;

GM_OFI_DEFINE_SUB(NdiView, View);
GM_OFI_PARAM2(NdiView, name, std::string, setName);
GM_OFI_PARAM2(NdiView, resolution, gmCore::size2, setResolution);
GM_OFI_PARAM2(NdiView, showUyvy, bool, setShowUyvy);
GM_OFI_POINTER2(NdiView, view, gmGraphics::View, addView);

namespace {
struct NdiX {
  static std::shared_ptr<NdiX> get() {
    static std::weak_ptr<NdiX> singleton;
    if (auto ptr = singleton.lock()) return ptr;

    std::shared_ptr<NdiX> ptr{new NdiX()};
    singleton = ptr;
    return ptr;
  }

  const NDIlib_v6 *lib = nullptr;

  ~NdiX() {
    if (lib) lib->NDIlib_destroy();
  }

private:
  std::shared_ptr<gmCore::ImportLibrary> ndi_importer;

  NdiX() : ndi_importer(new gmCore::ImportLibrary()) {
    ndi_importer->setLibrary(NDILIB_LIBRARY_NAME);
    ndi_importer->setPrefix("");
    ndi_importer->setSuffix("");
    ndi_importer->setDbgSuffix("");
    ndi_importer->initialize();

    if (!ndi_importer->isLoaded())
      throw gmCore::RuntimeException("Could not load NDI runtime");

    typedef const NDIlib_v6 *(*proc_NDIlib_v6_load)(void);
    lib = ((proc_NDIlib_v6_load)ndi_importer->getSymbol("NDIlib_v6_load"))();
    if (!lib) throw gmCore::RuntimeException("Could not load NDI functions");

    lib->NDIlib_initialize();
  }
};
}

struct NdiView::Impl {

  Impl();
  ~Impl();

  bool initialize();

  std::optional<gmCore::size2> resolution;
  OffscreenRenderTargets render_target;

  static const std::string rgba_fragment_code;
  RasterProcessor rgba_raster_processor;

  static const std::string uyvy_fragment_code;
  RasterProcessor uyvy_raster_processor;

  PixelPacker pixel_packer;
  size_t ndi_target_idx = 0;
  std::shared_ptr<PixelPacker::DataTransfer> last_data_transfer;

  std::vector<std::shared_ptr<View>> views;

  std::shared_ptr<NdiX> ndi;
  NDIlib_send_instance_t ndi_sender = 0;

  std::string ndi_name = "Gramods";
  std::optional<std::string> ndi_group;
  bool show_uyvy = false;
  std::optional<Eigen::Matrix4f> M_yuv;

  bool is_setup = false;
  bool is_functional = false;

  void renderFullPipeline(ViewSettings settings);
};

NdiView::NdiView() : _impl(std::make_unique<Impl>()) {}

NdiView::Impl::Impl() { ndi = NdiX::get(); }

NdiView::Impl::~Impl() {
  pixel_packer.executeAndUnmapAll();
  if (ndi_sender) {
    ndi->lib->NDIlib_send_send_video_async_v2(ndi_sender, nullptr);
    ndi->lib->NDIlib_send_destroy(ndi_sender);
  }
  if (last_data_transfer) last_data_transfer->is_in_use = false;
  last_data_transfer.reset();
}

void NdiView::initialize() {
  if (_impl->initialize()) View::initialize();
}

bool NdiView::Impl::initialize() {
  NDIlib_send_create_t send_create;
  send_create.p_ndi_name = ndi_name.c_str();
  send_create.p_groups = ndi_group ? ndi_group->c_str() : NULL;
  send_create.clock_video = true;
  send_create.clock_audio = false;

  ndi_sender = ndi->lib->NDIlib_send_create(&send_create);
  return ndi_sender != nullptr;
}

const std::string NdiView::Impl::rgba_fragment_code =
    R"lang=glsl(
#version 330 core

uniform sampler2D tex;

in vec2 position;

out vec4 fragColor;

void main() {

  vec3 rgb = texture(tex, position * 0.5 + 0.5).rgb;

  fragColor = vec4(rgb, 1);
}
)lang=glsl";

const std::string NdiView::Impl::uyvy_fragment_code =
    R"lang=glsl(
#version 330 core

uniform sampler2D tex;
uniform mat4 M_yuv;

in vec2 position;

out vec4 fragColor;

void main() {
  ivec2 pt = ivec2(gl_FragCoord.xy);
  pt.y = textureSize(tex, 0).y -1 -pt.y;

  vec3 rgb0 = texelFetch(tex, pt * ivec2(2, 1) + ivec2(0, 0), 0).rgb;
  vec3 rgb1 = texelFetch(tex, pt * ivec2(2, 1) + ivec2(1, 0), 0).rgb;

  vec3 yuv0 = (M_yuv * vec4(rgb0, 1)).xyz;
  vec3 yuv1 = (M_yuv * vec4(rgb1, 1)).xyz;

  float U = (yuv0.y + yuv1.y) * 0.5;
  float V = (yuv0.z + yuv1.z) * 0.5;

  fragColor = vec4(V, yuv0.x, U, yuv1.x) * vec4(0.87843137254901960784, 0.85882352941176470588, 0.87843137254901960784, 0.85882352941176470588) + 0.0625;
}
)lang=glsl";

void NdiView::renderFullPipeline(ViewSettings settings) {
  populateViewSettings(settings);
  _impl->renderFullPipeline(settings);
}

void NdiView::Impl::renderFullPipeline(ViewSettings settings) {

  if (views.empty()) {
    GM_RUNONCE(GM_ERR("NdiView", "No view to send."));
    return;
  }

  if (!resolution) {
    std::array<GLint, 4> viewport;
    glGetIntegerv(GL_VIEWPORT, viewport.data());
    resolution = {size_t(viewport[2]), size_t(viewport[3])};
  }

  if ((*resolution)[0] == 0 || (*resolution)[1] == 0) {
    GM_RUNONCE(GM_ERR("NdiView", "Cannot create zero size view (0x0)"));
    return;
  }

  if ((*resolution)[0] % 2) {
    GM_RUNONCE(GM_ERR(
        "NdiView", "Cannot do UYVY with a width that is not multiple of two"));
    return;
  }

  if (!is_setup) {
    is_setup = true;
    rgba_raster_processor.setFragmentCode(rgba_fragment_code);
    uyvy_raster_processor.setFragmentCode(uyvy_fragment_code);
    if (render_target.init(3) && //
        pixel_packer.init(2) &&
        rgba_raster_processor.init() && uyvy_raster_processor.init())
      is_functional = true;
  }

  if (!is_functional) {
    GM_RUNONCE(GM_ERR("NdiView", "Dysfunctional internal GL workings."));
    return;
  }

  auto t0 = std::chrono::steady_clock::now();

  // Render child views to RGBA target

  render_target.push();
  render_target.bind(*resolution, 0);

  for (auto view : views)
    view->renderFullPipeline(settings);

  render_target.pop();

  auto t1 = std::chrono::steady_clock::now();

  // Render RGBA to UYVY target

  static const GLint tex = 0;
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_BLEND);

  GLuint uyvy_program_id = uyvy_raster_processor.getProgramId();
  glUseProgram(uyvy_program_id);
  glUniform1iv(glGetUniformLocation(uyvy_program_id, "tex"), 1, &tex);

  // From NDI documentation:
  // SD resolutions: BT.601
  // HD resolutions (xres>720 || yres>576): Rec.709
  // UHD resolutions (xres>1920 || yres>1080): Rec.2020
  if (M_yuv) {
    glUniformMatrix4fv(glGetUniformLocation(uyvy_program_id, "M_yuv"), //
                       1, false, M_yuv.value().data());

  } else if (resolution.value()[0] > 1920 || resolution.value()[1] > 1080) {
    static Eigen::Matrix4f BT2020                              //
        {(Eigen::Matrix4f() << 0.2627f, 0.6780f, 0.0593f, 0.f, //
          -0.13963f, -0.36037f, 0.5f, 0.5f,                    //
          0.5f, -0.459786f, -0.040214f, 0.5f,                  //
          0.f, 0.f, 0.f, 1.f)
             .finished()};
    glUniformMatrix4fv(glGetUniformLocation(uyvy_program_id, "M_yuv"), //
                       1, false, BT2020.data());

  } else if (resolution.value()[0] > 720 || resolution.value()[1] > 576) {
    static Eigen::Matrix4f BT709                               //
        {(Eigen::Matrix4f() << 0.2126f, 0.7152f, 0.0722f, 0.f, //
          -0.1146f, -0.3854f, 0.5f, 0.5f,                      //
          0.5f, -0.4542f, -0.0458f, 0.5f,                      //
          0.f, 0.f, 0.f, 1.f)
             .finished()};
    glUniformMatrix4fv(glGetUniformLocation(uyvy_program_id, "M_yuv"), //
                       1, false, BT709.data());

  } else {
    static Eigen::Matrix4f BT601                           //
        {(Eigen::Matrix4f() << 0.299f, 0.587f, 0.114, 0.f, //
          -0.1687f, -0.3313f, 0.5f, 0.5f,                  //
          0.5f, -0.4187f, -0.0813f, 0.5f,                  //
          0.f, 0.f, 0.f, 1.f)
             .finished()};
    glUniformMatrix4fv(glGetUniformLocation(uyvy_program_id, "M_yuv"), //
                       1, false, BT601.data());
  }

  //< 0 is RGB buffer while 1 and 2 are NDI buffers
  ndi_target_idx = 1 + (ndi_target_idx % 2);
  render_target.push();
  render_target.bind({resolution.value()[0] / 2, resolution.value()[1]},
                     ndi_target_idx);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, render_target.getTexId(0));

  uyvy_raster_processor.run();

  // (Postpone render_target.pop() until we have read the data)

  auto t2 = std::chrono::steady_clock::now();

  // Read off pixels and send to NDI

  GLint size[4];
  glGetIntegerv(GL_VIEWPORT, size);

  int width = size[2];
  int height = size[3];

  {
    GLint current;
    glGetIntegerv(GL_DRAW_BUFFER, &current);
    glReadBuffer(current);
  }

  NDIlib_video_frame_v2_t frame{};
  frame.xres = 2 * width;
  frame.yres = height;
  frame.FourCC = NDIlib_FourCC_type_UYVY;
  frame.line_stride_in_bytes = 4 * width;
  frame.frame_rate_N = 60;
  frame.frame_rate_D = 1;

  pixel_packer.execute();
  pixel_packer.bind(
      width * height * 4,
      [frame, this](std::shared_ptr<PixelPacker::DataTransfer> tr) {
        tr->is_in_use = true;
        NDIlib_video_frame_v2_t my_frame = frame;
        my_frame.p_data = tr->data;
        ndi->lib->NDIlib_send_send_video_async_v2(ndi_sender, &my_frame);
        if (last_data_transfer) [[likely]]
          last_data_transfer->is_in_use = false;
        last_data_transfer = tr;
      });
  glReadPixels(size[0], size[1], width, height, GL_BGRA, GL_UNSIGNED_BYTE, 0);
  pixel_packer.unbind();

  render_target.pop();

  auto t3 = std::chrono::steady_clock::now();

  auto t4 = std::chrono::steady_clock::now();

  typedef std::chrono::duration<double, std::ratio<1>> d_seconds;
  auto dt1 = std::chrono::duration_cast<d_seconds>(t1 - t0);
  auto dt2 = std::chrono::duration_cast<d_seconds>(t2 - t1);
  auto dt3 = std::chrono::duration_cast<d_seconds>(t3 - t2);
  auto dt4 = std::chrono::duration_cast<d_seconds>(t4 - t3);

  GM_DBG2("NdiView", "Rendered ("
                         << int(1e3 * dt1.count() + 0.8) << " ms), converted ("
                         << int(1e3 * dt2.count() + 0.8) << " ms), read ("
                         << int(1e3 * dt3.count() + 0.8) << " ms) and sent ("
                         << int(1e3 * dt4.count() + 0.8) << " ms) image");

  // Render RGBA to parent view

  GLuint rgba_program_id = rgba_raster_processor.getProgramId();
  glUseProgram(rgba_program_id);
  glUniform1iv(glGetUniformLocation(rgba_program_id, "tex"), 1, &tex);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D,
                render_target.getTexId(show_uyvy ? ndi_target_idx : 0));

  rgba_raster_processor.run();

  glBindTexture(GL_TEXTURE_2D, 0);
}

void NdiView::setResolution(gmCore::size2 res) { _impl->resolution = res; }

void NdiView::setName(std::string name) { _impl->ndi_name = name; }
void NdiView::setGroup(std::string group) { _impl->ndi_group = group; }
void NdiView::setShowUyvy(bool on) { _impl->show_uyvy = on; }
void NdiView::setYuvMatrix(Eigen::Matrix4f M) { _impl->M_yuv = M; }

void NdiView::addView(std::shared_ptr<View> view) {
  if (!view) throw gmCore::InvalidArgument("null not allowed");
  _impl->views.push_back(view);
}

void NdiView::traverse(Visitor *visitor) {
  for (auto &v : _impl->views) v->accept(visitor);
}

END_NAMESPACE_GMGRAPHICS;

#endif
