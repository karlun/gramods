
#include <gmGraphics/OffscreenRenderTargets.hh>

#include <gmGraphics/GLUtils.hh>
#include <gmCore/Console.hh>

#include <stack>

BEGIN_NAMESPACE_GMGRAPHICS;

struct OffscreenRenderTargets::Impl {

  ~Impl();

  bool is_functional = false;

  std::vector<GLuint> fb_id;
  std::vector<GLuint> tex_id;
  GLuint rb_depth_id = 0;

  bool use_powers_of_two = false;
  std::vector<std::array<size_t, 2>> tex_size;

  std::stack<std::array<GLint, 4>> viewport_stack;
  std::stack<GLint> target_framebuffer_stack;

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

  GM_VINF("OffscreenRenderTargets", "Creating buffers and textures");
  fb_id.resize(count, 0);
  tex_id.resize(count, 0);
  tex_size.resize(count, { 0, 0 });
  glGenFramebuffers(count, fb_id.data());
  glGenTextures(count, tex_id.data());
  glGenRenderbuffers(1, &rb_depth_id);

  for (size_t idx = 0; idx < count; ++idx) {
    glBindFramebuffer(GL_FRAMEBUFFER, fb_id[idx]);
    glBindTexture(GL_TEXTURE_2D, tex_id[idx]);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 32, 32, 0,GL_RGB, GL_UNSIGNED_BYTE, 0);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex_id[idx], 0);
  }

  glBindRenderbuffer(GL_RENDERBUFFER, rb_depth_id);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32F, 32, 32);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rb_depth_id);

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

  size_t width, height;
  if (use_powers_of_two) {
    width = GLUtils::nextPowerOfTwo(vwidth);
    height = GLUtils::nextPowerOfTwo(vheight);
  } else {
    width = vwidth;
    height = vheight;
  }
  tex_size[idx] = { width, height };

  glBindFramebuffer(GL_FRAMEBUFFER, fb_id[idx]);

  glBindTexture(GL_TEXTURE_2D, tex_id[idx]);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
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

  GLint target_framebuffer;
  glGetIntegerv(GL_FRAMEBUFFER_BINDING, &target_framebuffer);
  target_framebuffer_stack.push(target_framebuffer);

  std::array<GLint, 4> viewport;
  glGetIntegerv(GL_VIEWPORT, viewport.data());
  viewport_stack.push(viewport);

}

void OffscreenRenderTargets::pop() {
  _impl->pop();
}

void OffscreenRenderTargets::Impl::pop() {
  GM_VINF("OffscreenRenderTargets", "finalizing");

  auto target_framebuffer = target_framebuffer_stack.top();
  glBindFramebuffer(GL_FRAMEBUFFER, target_framebuffer);
  target_framebuffer_stack.pop();

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
