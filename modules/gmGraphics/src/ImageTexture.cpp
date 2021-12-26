
#include <gmGraphics/ImageTexture.hh>

#ifdef gramods_ENABLE_FreeImage

#include <gmGraphics/FreeImage.hh>
#include <gmCore/Console.hh>
#include <gmCore/ExitLock.hh>

#include <FreeImage.h>

#include <regex>
#include <stdlib.h>


BEGIN_NAMESPACE_GMGRAPHICS;

GM_OFI_DEFINE(ImageTexture);
GM_OFI_PARAM2(ImageTexture, file, std::filesystem::path, setFile);
GM_OFI_PARAM2(ImageTexture, range, gmCore::size2, setRange);
GM_OFI_PARAM2(ImageTexture, autoRange, bool, setAutoRange);
GM_OFI_PARAM2(ImageTexture, loop, bool, setLoop);
GM_OFI_PARAM2(ImageTexture, exit, bool, setExit);

struct ImageTexture::Impl {

  Impl();

  GLuint update(size_t frame_number, Eye eye);
  void update(clock::time_point t);
  void findRange();
  bool loadImage(long int frame = 0);

  GLuint texture_id = 0;
  size_t texture_frame = std::numeric_limits<size_t>::max();
  bool fail = false;

  std::filesystem::path file = {};
  gmCore::size2 animation_range = {0, 0};
  bool auto_range = false;
  long int animation_frame = -1;
  bool animate = false;
  bool do_loop = false;
  bool do_exit = false;

  std::shared_ptr<FreeImage> free_image;
};

ImageTexture::ImageTexture()
  : _impl(new Impl) {}

void ImageTexture::initialize() {
  Texture::initialize();
}

void ImageTexture::setFile(std::filesystem::path file) {
  _impl->file = file;
}

void ImageTexture::setRange(gmCore::size2 range) {
  _impl->animation_range = range;
  _impl->animation_frame = range[0] - 1;
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

ImageTexture::Impl::Impl() {
  free_image = FreeImage::get();
}

GLuint ImageTexture::updateTexture(size_t frame_number, Eye eye) {
  return _impl->update(frame_number, eye);
}

GLuint ImageTexture::Impl::update(size_t frame_number, Eye eye) {

  if (!animate) {
    if (!texture_id) {
      GM_DBG2("ImageTexture", "Loading image");
      fail = !loadImage();
    }
    if (fail) return 0;
    return texture_id;
  }

  if (texture_frame != frame_number) {
    GM_DBG2("ImageTexture",
            "Loading animation frame " << animation_frame << ".");
    texture_frame = frame_number;
    fail = !loadImage(animation_frame);
  }

  if (fail) return 0;
  return texture_id;
}

void ImageTexture::update(clock::time_point t) {
  _impl->update(t);
}

void ImageTexture::Impl::update(clock::time_point) {

  if (!animate) return;

  if (auto_range) {
    findRange();
    auto_range = false;
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
    auto exit_lock = gmCore::ExitLock::get();
    if (exit_lock) exit_lock->requestExit(0);
  }
}

void ImageTexture::Impl::findRange() {

  size_t range_min = std::numeric_limits<size_t>::max();
  size_t range_max = std::numeric_limits<size_t>::min();

  const std::string filename = file.filename();
  const auto folder =
      file.has_parent_path() ? file.parent_path() : std::filesystem::path(".");

  const std::regex regex1("([^%]*)%[^d]*d(.*)");
  std::smatch match1;
  if (!std::regex_match(filename, match1, regex1) || match1.size() != 3) {
    GM_ERR("ImageTexture",
           "Could not autodetect range of current filename '"
               << filename << "' (in folder '" << folder << "')");
    animate = false;
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
    return;
  }

  GM_DBG1("ImageTexture",
          "Automatically determined animation range " << range_min << " - "
                                                      << range_max << ".");
  animation_range = {range_min, range_max};
  animation_frame = range_min - 1;
}

bool ImageTexture::Impl::loadImage(long int frame) {

  size_t filename_size = snprintf(nullptr, 0, file.u8string().c_str(), frame) + 1;
  std::vector<char> filename(filename_size + 1);
  snprintf(
      filename.data(), filename_size, file.u8string().c_str(), frame);

	FREE_IMAGE_FORMAT image_format = FreeImage_GetFileType(filename.data(), 0);
	if(image_format == FIF_UNKNOWN)
		image_format = FreeImage_GetFIFFromFilename(filename.data());
	if(image_format == FIF_UNKNOWN) {
    GM_ERR("ImageTexture", "Unknown image file type of file '" << filename.data() << "'");
		return false;
  }

  if (!FreeImage_FIFSupportsReading(image_format)) {
    GM_ERR("ImageTexture", "No read support for image '" << filename.data() << "'");
		return false;
  }

	FIBITMAP *image = FreeImage_Load(image_format, filename.data());
	if(!image) {
    GM_ERR("ImageTexture", "Could not load image '" << filename.data() << "'");
		return false;
  }

  FREE_IMAGE_TYPE image_type = FreeImage_GetImageType(image);
  GLenum gl_format;
  GLenum gl_type;
  switch (image_type) {
  case FIT_BITMAP:
    switch (FreeImage_GetBPP(image)) {
    case 8:
      gl_format = GL_RED;
      gl_type = GL_UNSIGNED_BYTE;
      break;
    case 24:
      gl_format = GL_BGR;
      gl_type = GL_UNSIGNED_BYTE;
      break;
    case 32:
      gl_format = GL_BGRA;
      gl_type = GL_UNSIGNED_BYTE;
      break;
    default:
      GM_ERR("ImageTexture", "Unsupported image format (" << FreeImage_GetBPP(image) << " bits per pixel)");
      FreeImage_Unload(image);
      return false;
    }
    break;
  case FIT_UINT16:
    gl_format = GL_RED;
    gl_type = GL_UNSIGNED_SHORT;
    break;
	case FIT_INT16:
    gl_format = GL_RED;
    gl_type = GL_SHORT;
    break;
	case FIT_UINT32:
    gl_format = GL_RED;
    gl_type =  GL_UNSIGNED_INT;
    break;
	case FIT_INT32:
    gl_format = GL_RED;
    gl_type = GL_INT;
    break;
	case FIT_FLOAT:
    gl_format = GL_RED;
    gl_type = GL_FLOAT;
    break;
	case FIT_RGB16:
    gl_format = GL_RGB;
    gl_type = GL_SHORT;
    break;
	case FIT_RGBA16:
    gl_format = GL_RGBA;
    gl_type = GL_SHORT;
    break;
	case FIT_RGBF:
    gl_format = GL_RGB;
    gl_type = GL_FLOAT;
    break;
	case FIT_RGBAF:
    gl_format = GL_RGBA;
    gl_type = GL_FLOAT;
    break;
  default:
    GM_ERR("ImageTexture", "Unknown pixel type (" << image_type << ") of image '" << filename.data() << "'");
    FreeImage_Unload(image);
    return false;
  }

	BYTE* image_data = FreeImage_GetBits(image);
	unsigned int image_width = FreeImage_GetWidth(image);
	unsigned int image_height = FreeImage_GetHeight(image);
	if((image_data == 0) || (image_width == 0) || (image_height == 0)) {
    FreeImage_Unload(image);
		return false;
  }

  if (!texture_id) {
    glGenTextures(1, &texture_id);
  }

  glBindTexture(GL_TEXTURE_2D, texture_id);
  glTexImage2D(GL_TEXTURE_2D,
               0, GL_RGBA, image_width, image_height,
               0, gl_format, gl_type, image_data);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glBindTexture(GL_TEXTURE_2D, 0);

	FreeImage_Unload(image);

  GM_DBG2("ImageTexture", "Loaded"
          << " image " << filename.data()
          << " " << image_width << "x" << image_height
          );

	return true;
}

END_NAMESPACE_GMGRAPHICS;

#endif
