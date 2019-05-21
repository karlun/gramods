
#include <gmGraphics/OffscreenRenderTarget.hh>

#include <gmGraphics/GLUtils.hh>
#include <gmCore/Console.hh>

#include <stack>

BEGIN_NAMESPACE_GMGRAPHICS;

struct OffscreenRenderTarget::Impl {

  ~Impl();

  bool is_functional = false;

  GLuint fb_id = 0;
  GLuint tex_id = 0;
  GLuint rb_depth_id = 0;

  std::stack<std::array<GLint, 4>> viewport_stack;
  std::stack<GLint> target_framebuffer_stack;

  bool init();

  void push();
  void pop();

  void bind(size_t width, size_t height);

  void teardown();
};

OffscreenRenderTarget::OffscreenRenderTarget()
  : _impl(std::make_unique<OffscreenRenderTarget::Impl>()) {}

OffscreenRenderTarget::~OffscreenRenderTarget() {}

OffscreenRenderTarget::Impl::~Impl() {
  teardown();
}

bool OffscreenRenderTarget::init() {
  return _impl->init();
}

bool OffscreenRenderTarget::Impl::init() {
  is_functional = false;

  GM_VINF("OffscreenRenderTarget", "Creating buffers and textures");
  glGenFramebuffers(1, &fb_id);
  glGenTextures(1, &tex_id);
  glGenRenderbuffers(1, &rb_depth_id);

  glBindFramebuffer(GL_FRAMEBUFFER, fb_id);
  glBindTexture(GL_TEXTURE_2D, tex_id);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 32, 32, 0,GL_RGB, GL_UNSIGNED_BYTE, 0);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

  glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex_id, 0);

  glBindRenderbuffer(GL_RENDERBUFFER, rb_depth_id);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32F, 32, 32);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rb_depth_id);

  if (!GLUtils::check_framebuffer())
    return false;

  is_functional = true;
  return true;
}

void OffscreenRenderTarget::Impl::teardown() {
  is_functional = false;

  if (fb_id) glDeleteFramebuffers(1, &fb_id);
  if (tex_id) glDeleteTextures(1, &tex_id);
  if (rb_depth_id) glDeleteRenderbuffers(1, &rb_depth_id);

  fb_id = 0;
  tex_id = 0;
  rb_depth_id = 0;
}

void OffscreenRenderTarget::bind(size_t width, size_t height) {
  _impl->bind(width, height);
}

GLuint OffscreenRenderTarget::getTexId() {
  return _impl->tex_id;
}

void OffscreenRenderTarget::Impl::bind(size_t width, size_t height) {
  if (!is_functional)
    return;

  glBindFramebuffer(GL_FRAMEBUFFER, fb_id);

  glBindTexture(GL_TEXTURE_2D, tex_id);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
  glBindTexture(GL_TEXTURE_2D, 0);

  glBindRenderbuffer(GL_RENDERBUFFER, rb_depth_id);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32F, width, height);
  glBindRenderbuffer(GL_RENDERBUFFER, 0);

  glViewport(0, 0, width, height);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void OffscreenRenderTarget::push() {
  _impl->push();
}

void OffscreenRenderTarget::Impl::push() {

  GLint target_framebuffer;
  glGetIntegerv(GL_FRAMEBUFFER_BINDING, &target_framebuffer);
  target_framebuffer_stack.push(target_framebuffer);

  std::array<GLint, 4> viewport;
  glGetIntegerv(GL_VIEWPORT, viewport.data());
  viewport_stack.push(viewport);

}

void OffscreenRenderTarget::pop() {
  _impl->pop();
}

void OffscreenRenderTarget::Impl::pop() {
  GM_VINF("OffscreenRenderTarget", "finalizing");

  auto target_framebuffer = target_framebuffer_stack.top();
  glBindFramebuffer(GL_FRAMEBUFFER, target_framebuffer);
  target_framebuffer_stack.pop();

  auto viewport = viewport_stack.top();
  glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
  viewport_stack.pop();
}

END_NAMESPACE_GMGRAPHICS;
