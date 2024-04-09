
#include <gmGraphics/OffscreenRenderTargets.hh>

#include <gmGraphics/GLUtils.hh>
#include <gmCore/Console.hh>

#include <array>
#include <stack>

BEGIN_NAMESPACE_GMGRAPHICS;

struct OffscreenRenderTargets::Impl {

  ~Impl();

  bool is_functional = false;

  std::vector<GLuint> fb_id;
  std::vector<GLuint> tex_id;
  GLuint rb_depth_id = 0;

  bool use_powers_of_two = false;
  GLenum pixel_format = GL_RGBA8;
  std::vector<std::array<size_t, 2>> tex_size;
  bool use_linear = false;

  std::stack<std::array<GLint, 4>> viewport_stack;
  std::stack<GLint> target_framebuffer_stack;
  std::stack<GLint> target_renderbuffer_stack;

  bool init(size_t count);

  void push();
  void pop();

  void bind(size_t width, size_t height, size_t idx);

  void teardown();
};

OffscreenRenderTargets::OffscreenRenderTargets()
  : _impl(std::make_unique<OffscreenRenderTargets::Impl>()) {}

OffscreenRenderTargets::~OffscreenRenderTargets() {}

OffscreenRenderTargets::Impl::~Impl() {
  teardown();
}

bool OffscreenRenderTargets::init(size_t count) {
  return _impl->init(count);
}

bool OffscreenRenderTargets::Impl::init(size_t count) {
  is_functional = false;

  if (count < 1 || count > 10)
    return false;

  GM_DBG2("OffscreenRenderTargets", "Creating buffers and textures");

  GLint old_framebuffer, old_renderbuffer;
  glGetIntegerv(GL_FRAMEBUFFER_BINDING, &old_framebuffer);
  glGetIntegerv(GL_RENDERBUFFER_BINDING, &old_renderbuffer);

  fb_id.resize(count, 0);
  tex_id.resize(count, 0);
  tex_size.resize(count, { 0, 0 });
  glGenFramebuffers(count, fb_id.data());
  glGenTextures(count, tex_id.data());
  glGenRenderbuffers(1, &rb_depth_id);

  glBindRenderbuffer(GL_RENDERBUFFER, rb_depth_id);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32F, 32, 32);

  for (size_t idx = 0; idx < count; ++idx) {
    glBindFramebuffer(GL_FRAMEBUFFER, fb_id[idx]);
    glBindTexture(GL_TEXTURE_2D, tex_id[idx]);

    glTexImage2D(GL_TEXTURE_2D, 0, pixel_format, 32, 32, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);

    if (use_linear) {
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    } else {
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    }

    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex_id[idx], 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rb_depth_id);
  }
  glBindRenderbuffer(GL_RENDERBUFFER, old_renderbuffer);
  glBindFramebuffer(GL_FRAMEBUFFER, old_framebuffer);

  if (!GLUtils::check_framebuffer())
    return false;

  is_functional = true;
  return true;
}

void OffscreenRenderTargets::Impl::teardown() {
  is_functional = false;

  if (fb_id.size()) glDeleteFramebuffers(fb_id.size(), fb_id.data());
  if (tex_id.size()) glDeleteTextures(tex_id.size(), tex_id.data());
  if (rb_depth_id) glDeleteRenderbuffers(1, &rb_depth_id);

  fb_id.clear();
  tex_id.clear();
  rb_depth_id = 0;
}

void OffscreenRenderTargets::bind(gmCore::size2 res, size_t idx) {
  _impl->bind(res[0], res[1], idx);
}

void OffscreenRenderTargets::bind(size_t width, size_t height, size_t idx) {
  _impl->bind(width, height, idx);
}

GLuint OffscreenRenderTargets::getTexId(size_t idx) {
  if (idx >= _impl->tex_id.size()) return 0;
  return _impl->tex_id[idx];
}

void OffscreenRenderTargets::Impl::bind(size_t vwidth, size_t vheight, size_t idx) {

  if (!is_functional)
    return;

  if (idx >= fb_id.size())
    return;

  if (!vwidth && !viewport_stack.empty()) vwidth = viewport_stack.top()[2];
  if (!vheight && !viewport_stack.empty()) vheight = viewport_stack.top()[3];

  size_t width, height;
  if (use_powers_of_two) {
    width = GLUtils::nextPowerOfTwo(vwidth);
    height = GLUtils::nextPowerOfTwo(vheight);
  } else {
    width = vwidth;
    height = vheight;
  }
  tex_size[idx] = { width, height };
  GM_DBG2("OffscreenRenderTargets", "Bind " << idx << " with size " << width << "x" << height);

  glBindFramebuffer(GL_FRAMEBUFFER, fb_id[idx]);

  glBindTexture(GL_TEXTURE_2D, tex_id[idx]);
  glTexImage2D(GL_TEXTURE_2D, 0, pixel_format, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
  glBindTexture(GL_TEXTURE_2D, 0);

  glBindRenderbuffer(GL_RENDERBUFFER, rb_depth_id);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32F, width, height);
  glBindRenderbuffer(GL_RENDERBUFFER, 0);

  glViewport(0, 0, vwidth, vheight);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void OffscreenRenderTargets::push() {
  _impl->push();
}

void OffscreenRenderTargets::Impl::push() {

  GLint param;
  glGetIntegerv(GL_FRAMEBUFFER_BINDING, &param);
  target_framebuffer_stack.push(param);

  glGetIntegerv(GL_RENDERBUFFER_BINDING, &param);
  target_renderbuffer_stack.push(param);

  std::array<GLint, 4> viewport;
  glGetIntegerv(GL_VIEWPORT, viewport.data());
  viewport_stack.push(viewport);

}

void OffscreenRenderTargets::pop() {
  _impl->pop();
}

void OffscreenRenderTargets::Impl::pop() {
  GM_DBG2("OffscreenRenderTargets", "finalizing");

  auto target_framebuffer = target_framebuffer_stack.top();
  glBindFramebuffer(GL_FRAMEBUFFER, target_framebuffer);
  target_framebuffer_stack.pop();

  auto target_renderbuffer = target_renderbuffer_stack.top();
  glBindRenderbuffer(GL_RENDERBUFFER, target_renderbuffer);
  target_renderbuffer_stack.pop();

  auto viewport = viewport_stack.top();
  glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
  viewport_stack.pop();
}

void OffscreenRenderTargets::setUsePowersOfTwo(bool on) {
  _impl->use_powers_of_two = on;
}

bool OffscreenRenderTargets::getUsePowersOfTwo() {
  return _impl->use_powers_of_two;
}

void OffscreenRenderTargets::setLinearInterpolation(bool on) {
  _impl->use_linear = on;
}

void OffscreenRenderTargets::setPixelFormat(GLenum format) {
  _impl->pixel_format = format;
}

GLenum OffscreenRenderTargets::getPixelFormat() {
  return _impl->pixel_format;
}

void OffscreenRenderTargets::getTextureSize(size_t &width, size_t &height, size_t idx) {
  if (idx >= _impl->tex_size.size()) {
    width = 0;
    height = 0;
  } else {
    width = _impl->tex_size[idx][0];
    height = _impl->tex_size[idx][1];
  }
}

END_NAMESPACE_GMGRAPHICS;
