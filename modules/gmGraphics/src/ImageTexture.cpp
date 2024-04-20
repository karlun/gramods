
#include <gmGraphics/ImageTexture.hh>

#ifdef gramods_ENABLE_FreeImage

#include <gmCore/Console.hh>
#include <gmCore/FreeImage.hh>
#include <gmCore/ExitException.hh>
#include <gmCore/FileResolver.hh>

#include <FreeImage.h>

#include <regex>
#include <stdlib.h>

#include <chrono>
#include <thread>
#include <condition_variable>

BEGIN_NAMESPACE_GMGRAPHICS;

GM_OFI_DEFINE(ImageTexture);
GM_OFI_PARAM2(ImageTexture, file, std::filesystem::path, setFile);
GM_OFI_PARAM2(ImageTexture, range, gmCore::size2, setRange);
GM_OFI_PARAM2(ImageTexture, autoRange, bool, setAutoRange);
GM_OFI_PARAM2(ImageTexture, loop, bool, setLoop);
GM_OFI_PARAM2(ImageTexture, exit, bool, setExit);
GM_OFI_PARAM2(ImageTexture, logProgress, bool, setLogProgress);

struct ImageTexture::Impl {

  Impl();
  ~Impl();

  GLuint update(size_t frame_number, Eye eye);
  void update();
  void findRange();
  static std::string getFrameFilename(std::string file, long int frame);
  static FIBITMAP *loadImage(std::string filename);
  bool setTexture(FIBITMAP *image, std::string filename);

  GLuint texture_id = 0;
  size_t texture_frame = std::numeric_limits<size_t>::max();
  bool fail = false;

  std::filesystem::path file = {};
  gmCore::size2 animation_range = {0, 0};
  bool auto_range = false;
  size_t animation_frame = std::numeric_limits<size_t>::max();
  bool animate = false;
  bool do_loop = false;
  bool do_exit = false;

  typedef std::chrono::steady_clock clock;
  typedef std::chrono::duration<double, std::ratio<1>> d_seconds;
  bool do_log_progress = false;
  clock::time_point last_print_time = clock::now();

  std::shared_ptr<gmCore::FreeImage> free_image;

  void load_process();

  bool load_process_alive = true;
  std::thread load_thread;
  std::condition_variable load_condition;
  std::mutex load_lock;
  size_t load_frame = std::numeric_limits<size_t>::max();
  std::string load_filename;
  FIBITMAP *load_image = nullptr;
};

ImageTexture::ImageTexture()
  : _impl(new Impl) {}
ImageTexture::~ImageTexture() {}

void ImageTexture::initialize() {
  Texture::initialize();
}

void ImageTexture::setFile(std::filesystem::path file) {
  _impl->file = file;
}

std::filesystem::path ImageTexture::getFile() { return _impl->file; }

void ImageTexture::setRange(gmCore::size2 range) {
  if (range[0] > range[1])
    throw gmCore::InvalidArgument(
        GM_STR("Invalid range: " << range[0] << " - " << range[1]));

  _impl->animation_range = range;

  if (_impl->animation_frame + 1 < range[0])
    _impl->animation_frame = range[0] - 1;

  if (_impl->animation_frame + 1 > range[1])
    _impl->animation_frame = range[1] - 1;

  _impl->animate = true;
}

void ImageTexture::setAutoRange(bool on) {
  _impl->auto_range = on;
  _impl->animate = true;
}

void ImageTexture::setLoop(bool on) {
  _impl->do_loop = on;
}

void ImageTexture::setExit(bool on) {
  _impl->do_exit = on;
}

void ImageTexture::setLogProgress(bool on) {
  _impl->do_log_progress = on;
}

ImageTexture::Impl::Impl() {
  free_image = gmCore::FreeImage::get();
  load_thread = std::thread([this]{ this->load_process(); });
}

ImageTexture::Impl::~Impl() {

  {
    std::unique_lock<std::mutex> guard(load_lock);
    load_process_alive = false;
    load_condition.notify_all();
  }

  if (!load_thread.joinable()) return;

  try {
    load_thread.join();
  } catch (const std::invalid_argument &e) {
    GM_DBG2("ImageTexture", "Could not join load thread: " << e.what() << ".");
  } catch (const std::system_error &e) {
    GM_WRN("ImageTexture",
           "Caught system_error while joining load thread. Code "
           << e.code() << " meaning " << e.what() << ".");
  }
}

GLuint ImageTexture::updateTexture(size_t frame_number, Eye eye) {
  return _impl->update(frame_number, eye);
}

GLuint ImageTexture::Impl::update(size_t frame_number, Eye eye) {

  if (fail) return 0;

  if (!animate) {
    if (texture_id) return texture_id;

    GM_DBG2("ImageTexture", "Loading image");
    auto filename = file.u8string();
    FIBITMAP *image = loadImage(filename);
    if (!image) {
      fail = true;
      return 0;
    }

    fail = !setTexture(image, filename);

    if (fail) return 0;
    return texture_id;
  }

  if (texture_frame == frame_number) return texture_id;

  GM_DBG2("ImageTexture",
          "Loading animation frame " << animation_frame << ".");

  std::unique_lock<std::mutex> guard(load_lock);

  if (load_frame != animation_frame) {
    GM_DBG2("ImageTexture",
            "Asynchroneous loading not triggered - triggering and waiting.");
    load_frame = animation_frame;
    load_filename = getFrameFilename(file.u8string(), load_frame);
    load_condition.notify_all();
    while (load_image == nullptr) {
      load_condition.wait_for(guard, std::chrono::seconds(1));
      if (!load_process_alive) return 0;
    }
  }

  fail = !setTexture(load_image, load_filename);
  load_image = nullptr;

  texture_frame = frame_number;

  auto next_frame = animation_frame + 1;
  if (next_frame <= animation_range[1]) {
    GM_DBG2("ImageTexture",
            "Triggering asynchroneous pre-loading of frame " << next_frame << ".");
    load_frame = next_frame;
    load_filename = getFrameFilename(file.u8string(), load_frame);
    load_condition.notify_all();
  } else if (do_loop) {
    GM_DBG2("ImageTexture",
            "Triggering asynchroneous pre-loading of frame " << next_frame << ".");
    load_frame = animation_range[0];
    load_filename = getFrameFilename(file.u8string(), load_frame);
    load_condition.notify_all();
  }

  if (fail) return 0;
  return texture_id;
}

void ImageTexture::update(clock::time_point, size_t) {
  _impl->update();
}

void ImageTexture::Impl::update() {

  if (!animate) return;

  if (auto_range) {
    findRange();
    auto_range = false;
  }

  if (do_log_progress) {
    auto current_time = clock::now();
    auto dt = std::chrono::duration_cast<d_seconds>(current_time - last_print_time);

    if (dt.count() > 2) {

      GM_INF("gm-load",
             "ImageTexture frame "
                 << (animation_frame - animation_range[0]) << " of "
                 << (animation_range[1] - animation_range[0]) << " ("
                 << animation_range[0] << " < " << animation_frame << " < "
                 << animation_range[1] << ") ≈ "
                 << int((100 * (animation_frame - animation_range[0])) /
                        (animation_range[1] - animation_range[0])));

      last_print_time = current_time;
    }
  }

  if ((size_t)++animation_frame <= animation_range[1]) return;

  if (do_loop) {
    GM_DBG2("ImageTexture", "Looping animation");
    animation_frame = animation_range[0];
    return;
  }

  GM_DBG2("ImageTexture", "Animation done");
  animate = false;

  if (do_exit) {
    throw gmCore::ExitException(0);
  }
}

void ImageTexture::Impl::findRange() {

  size_t range_min = std::numeric_limits<size_t>::max();
  size_t range_max = std::numeric_limits<size_t>::min();

  const std::string filename = file.filename().u8string();
  const auto folder =
      file.has_parent_path() ? file.parent_path() : std::filesystem::path(".");
  if (!std::filesystem::is_directory(folder)) {
    GM_ERR("ImageTexture",
           "Could not autodetect range of current filename '"
               << filename << "' because folder '" << folder.u8string()
               << "' is not a folder.");
    animate = false;
    fail = true;
    return;
  }

  const std::regex regex1("([^%]*)%[^d]*d(.*)");
  std::smatch match1;
  if (!std::regex_match(filename, match1, regex1) || match1.size() != 3) {
    GM_ERR("ImageTexture",
           "Could not autodetect range of current filename '"
               << filename << "' (in folder '" << folder.u8string() << "')");
    animate = false;
    fail = true;
    return;
  }

  const std::string pattern2 = GM_STR(match1[1] << "([0-9]+)" << match1[2]);
  const std::regex regex2(pattern2);
  std::smatch match2;
  for (const auto & entry : std::filesystem::directory_iterator(folder)) {
    const std::string filename = entry.path().filename().string();

    if (!entry.is_regular_file()) {
      GM_DBG3("ImageTexture", "Skipping non file entry '" << filename << "'.");
      continue;
    }

    if (!std::regex_match(filename, match2, regex2)) {
      GM_DBG3("ImageTexture",
              "No match for pattern '" << pattern2 << "' on file '" << filename
                                       << "'.");
      continue;
    }
    if (match2.size() != 2) {
      GM_WRN("ImageTexture",
             "Could not fully match pattern '" << pattern2 << "' on file '"
                                               << filename << "'.");
      continue;
    }

    size_t frame;
    if (!(std::stringstream(match2[1]) >> frame)) {
      GM_WRN("ImageTexture",
             "Could not parse frame number '" << match2[1] << "' on file '"
                                              << filename << "'.");
      continue;
    }

    range_min = std::min(range_min, frame);
    range_max = std::max(range_max, frame);
  }

  if (range_min == std::numeric_limits<size_t>::max()) {
    GM_ERR("ImageTexture",
           "Failed to automatically determine animation range");
    animate = false;
    fail = true;
    return;
  }

  GM_DBG1("ImageTexture",
          "Automatically determined animation range " << range_min << " - "
                                                      << range_max << ".");
  animation_range = {range_min, range_max};
  animation_frame = range_min - 1;
}

std::string ImageTexture::Impl::getFrameFilename(std::string file,
                                                 long int frame) {
  size_t filename_size = snprintf(nullptr, 0, file.c_str(), frame);
  std::vector<char> filename(filename_size + 1);
  snprintf(filename.data(), filename_size + 1, file.c_str(), frame);
  return std::string(filename.begin(), filename.end() - 1);
}

FIBITMAP *ImageTexture::Impl::loadImage(std::string filename) {
  try {
    filename =
        gmCore::FileResolver::getDefault()
            ->resolve(filename, gmCore::FileResolver::Check::ReadableFile)
            .u8string();
  } catch (gmCore::InvalidArgument &err) {
    GM_ERR("ImageTexture", err.what);
    return nullptr;
  }

	FREE_IMAGE_FORMAT image_format = FreeImage_GetFileType(filename.c_str(), 0);
	if(image_format == FIF_UNKNOWN)
		image_format = FreeImage_GetFIFFromFilename(filename.c_str());
	if(image_format == FIF_UNKNOWN) {
    GM_ERR("ImageTexture", "Unknown image file type of file '" << filename << "'");
		return nullptr;
  }

  if (!FreeImage_FIFSupportsReading(image_format)) {
    GM_ERR("ImageTexture", "No read support for image '" << filename << "'");
		return nullptr;
  }

	FIBITMAP *image = FreeImage_Load(image_format, filename.c_str());
	if(!image) {
    GM_ERR("ImageTexture", "Could not load image '" << filename << "'");
		return nullptr;
  }

  return image;
}

bool ImageTexture::Impl::setTexture(FIBITMAP *image, std::string filename) {

  FREE_IMAGE_TYPE image_type = FreeImage_GetImageType(image);
  GLenum gl_format;
  GLenum gl_type;
  std::string str_format = "";
  switch (image_type) {
  case FIT_BITMAP:
    switch (FreeImage_GetBPP(image)) {
    case 8:
      gl_format = GL_RED;
      gl_type = GL_UNSIGNED_BYTE;
      str_format = "u8 gray";
      break;
    case 24:
      gl_format = GL_BGR;
      gl_type = GL_UNSIGNED_BYTE;
      str_format = "u8 RGB";
      break;
    case 32:
      gl_format = GL_BGRA;
      gl_type = GL_UNSIGNED_BYTE;
      str_format = "u8 RGBA";
      break;
    default:
      GM_ERR("ImageTexture",
             "Unsupported image format (" << FreeImage_GetBPP(image)
                                          << " bits per pixel) of image '"
                                          << filename << "'");
      FreeImage_Unload(image);
      return false;
    }
    break;
  case FIT_UINT16:
    gl_format = GL_RED;
    gl_type = GL_UNSIGNED_SHORT;
    str_format = "u16 gray";
    break;
  case FIT_INT16:
    gl_format = GL_RED;
    gl_type = GL_SHORT;
    str_format = "s16 gray";
    break;
  case FIT_UINT32:
    gl_format = GL_RED;
    gl_type = GL_UNSIGNED_INT;
    str_format = "u32 gray";
    break;
  case FIT_INT32:
    gl_format = GL_RED;
    gl_type = GL_INT;
    str_format = "s32 gray";
    break;
  case FIT_FLOAT:
    gl_format = GL_RED;
    gl_type = GL_FLOAT;
    str_format = "f32 gray";
    break;
  case FIT_RGB16:
    gl_format = GL_RGB;
    gl_type = GL_SHORT;
    str_format = "s16 RGB";
    break;
  case FIT_RGBA16:
    gl_format = GL_RGBA;
    gl_type = GL_SHORT;
    str_format = "s16 RGBA";
    break;
  case FIT_RGBF:
    gl_format = GL_RGB;
    gl_type = GL_FLOAT;
    str_format = "f32 RGB";
    break;
  case FIT_RGBAF:
    gl_format = GL_RGBA;
    gl_type = GL_FLOAT;
    str_format = "f32 RGBA";
    break;
  default:
    GM_ERR("ImageTexture",
           "Unknown pixel type (" << image_type << ") of image '"
                                  << filename << "'");
    FreeImage_Unload(image);
    return false;
  }

  BYTE *image_data = FreeImage_GetBits(image);
  unsigned int image_width = FreeImage_GetWidth(image);
  unsigned int image_height = FreeImage_GetHeight(image);
  if ((image_data == 0) || (image_width == 0) || (image_height == 0)) {
    GM_ERR("ImageTexture",
           "No valid pixel data in image '" << filename << "'");
    FreeImage_Unload(image);
    return false;
  }

  if (!texture_id) { glGenTextures(1, &texture_id); }

  glBindTexture(GL_TEXTURE_2D, texture_id);
  glTexImage2D(GL_TEXTURE_2D,
               0, GL_RGBA, image_width, image_height,
               0, gl_format, gl_type, image_data);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glBindTexture(GL_TEXTURE_2D, 0);

  FreeImage_Unload(image);

  GM_DBG2("ImageTexture",
          "Loaded "
              << " image '" << filename << "' (" << image_width << "x"
          << image_height << " " << str_format << ")");

  return true;
}

void ImageTexture::Impl::load_process() {
  std::unique_lock<std::mutex> guard(load_lock);
  while(load_process_alive) {
    GM_DBG3("ImageTexture",
            "Asynchroneous loader sleeping.");
    load_condition.wait_for(guard, std::chrono::seconds(1));

    if (load_filename.empty()) continue;

    GM_DBG3("ImageTexture",
            "Asynchroneous loading triggered.");

    auto t0 = std::chrono::steady_clock::now();

    if (load_image != nullptr) FreeImage_Unload(load_image);
    load_image = loadImage(load_filename);
    load_filename.clear();

    auto t1 = std::chrono::steady_clock::now();

    typedef std::chrono::duration<double, std::ratio<1>> d_seconds;
    auto dt1 = std::chrono::duration_cast<d_seconds>(t1 - t0);
    GM_DBG2("ImageTexture",
            "Asynchroneous image load in " << int(1e3 * dt1.count() + 0.8)
                                           << " ms");
  }
}

END_NAMESPACE_GMGRAPHICS;

#endif
