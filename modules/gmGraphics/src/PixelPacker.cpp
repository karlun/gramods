
#include <gmGraphics/PixelPacker.hh>

#include <gmCore/Console.hh>
#include <gmCore/PreConditionViolation.hh>
#include <gmCore/RunOnce.hh>
#include <gmCore/RuntimeException.hh>

#include <gmGraphics/GLUtils.hh>

#include <vector>

BEGIN_NAMESPACE_GMGRAPHICS;

namespace {
struct PboData {
  size_t byte_count = 0;
  PixelPacker::callback_function function = {};
  std::shared_ptr<PixelPacker::DataTransfer> data_transfer =
      std::make_shared<PixelPacker::DataTransfer>();
};
}

struct PixelPacker::Impl {
  ~Impl();

  bool is_functional = false;
  std::vector<GLuint> pbo_id;
  std::vector<PboData> pbo_data;

  size_t bind_idx = 0;
  std::optional<size_t> execute_idx;

  bool init(size_t count);

  void bind(size_t byte_count, callback_function ufunc);
  void execute(bool unmap = false);
  void executeAndUnmapAll();

  void teardown();
};

PixelPacker::PixelPacker() : _impl(new Impl()) {}
PixelPacker::~PixelPacker() {}

PixelPacker::Impl::~Impl() {
  executeAndUnmapAll();
  teardown();
}

bool PixelPacker::init(size_t count) {
  return _impl->init(count);
}

bool PixelPacker::Impl::init(size_t count) {
  pbo_id.resize(count);
  glGenBuffers(pbo_id.size(), pbo_id.data());

  if (!GLUtils::isStateGood()) return false;

  for (size_t idx = 0; idx < count; ++idx) pbo_data.push_back({});

  is_functional = true;
  return true;
}

void PixelPacker::bind(size_t byte_count, callback_function ufunc) {
  _impl->bind(byte_count, ufunc);
}

void PixelPacker::Impl::bind(size_t byte_count, callback_function ufunc) {

  if (execute_idx && bind_idx == *execute_idx)
    throw gmCore::PreConditionViolation("Cannot bind; no PBO left");

  pbo_data[bind_idx].function = ufunc;

  glBindBuffer(GL_PIXEL_PACK_BUFFER, pbo_id[bind_idx]);

  if (pbo_data[bind_idx].data_transfer->data) {
    if (pbo_data[bind_idx].data_transfer->is_in_use)
      GM_RUNONCE(GM_WRN("PixelPacker", "Unmap stalled; still in use."));
    pbo_data[bind_idx].data_transfer->is_in_use.wait(true);
    if (!glUnmapBuffer(GL_PIXEL_PACK_BUFFER)) pbo_data[bind_idx].byte_count = 0;
    pbo_data[bind_idx].data_transfer->data = nullptr;
  }

  if (pbo_data[bind_idx].byte_count != byte_count) {
    glBufferData(GL_PIXEL_PACK_BUFFER, byte_count, 0, GL_STREAM_READ);
    pbo_data[bind_idx].byte_count = byte_count;
  }

  if (!execute_idx) execute_idx = bind_idx;
  bind_idx = (bind_idx + 1) % pbo_id.size();
}

void PixelPacker::unbind() { glBindBuffer(GL_PIXEL_PACK_BUFFER, 0); }

void PixelPacker::executeAndUnmapAll() {
  _impl->executeAndUnmapAll();
}

void PixelPacker::Impl::executeAndUnmapAll() {
  while (execute_idx) execute(true);
}

void PixelPacker::execute() {
  _impl->execute();
}

void PixelPacker::Impl::execute(bool unmap) {
  if (!execute_idx) return;

  glBindBuffer(GL_PIXEL_PACK_BUFFER, pbo_id[*execute_idx]);
  pbo_data[*execute_idx].data_transfer->data =
      (GLubyte *)glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY);
  if (!pbo_data[*execute_idx].data_transfer->data) {
    glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
    GLUtils::isStateGood("While mapping PBO: ");
    throw gmCore::RuntimeException("Could not map PBO; lost OpenGL context?");
  }

  pbo_data[*execute_idx].function(pbo_data[*execute_idx].data_transfer);
  pbo_data[*execute_idx].function = {};

  if (unmap) {
    pbo_data[bind_idx].data_transfer->is_in_use.wait(true);
    if (!glUnmapBuffer(GL_PIXEL_PACK_BUFFER))
      pbo_data[*execute_idx].byte_count = 0;
    pbo_data[bind_idx].data_transfer->data = nullptr;
  }

  glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);

  execute_idx = (*execute_idx + 1) % pbo_id.size();
  if (*execute_idx == bind_idx) execute_idx = std::nullopt;
}

void PixelPacker::Impl::teardown() {
  if (!pbo_id.empty()) glDeleteBuffers(pbo_id.size(), pbo_id.data());
  pbo_id.clear();
  pbo_data.clear();
  is_functional = false;
}

END_NAMESPACE_GMGRAPHICS;
