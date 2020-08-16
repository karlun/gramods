
#include <gmGraphics/ImageTexture.hh>

#ifdef gramods_ENABLE_FreeImage

#include <gmGraphics/FreeImage.hh>
#include <gmCore/Console.hh>

#include <FreeImage.h>
#include <stdlib.h>

BEGIN_NAMESPACE_GMGRAPHICS;

GM_OFI_DEFINE(ImageTexture);
GM_OFI_PARAM(ImageTexture, file, std::string, ImageTexture::setFile);
GM_OFI_PARAM(ImageTexture, range, gmTypes::size2, ImageTexture::setRange);
GM_OFI_PARAM(ImageTexture, loop, bool, ImageTexture::setLoop);
GM_OFI_PARAM(ImageTexture, exit, bool, ImageTexture::setExit);

struct ImageTexture::Impl {

  Impl();

  GLuint getGLTextureID() { return texture_id; }

  void update();
  void update(clock::time_point t);
  bool loadImage(std::string filename, long int frame = 0);

  GLuint texture_id = 0;
  std::string file = "";
  bool fail = false;
  gmTypes::size2 animation_range;
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

void ImageTexture::setFile(std::string file) {
  _impl->file = file;
}

void ImageTexture::setRange(gmTypes::size2 range) {
  _impl->animation_range = range;
  _impl->animation_frame = range[0] - 1;
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

void ImageTexture::Impl::update() {

  if (fail) return;

  if (animate) {
    GM_VINF("ImageTexture", "Animation frame " << animation_frame);
    fail = !loadImage(file, animation_frame);


  } else if (!texture_id) {
    fail = !loadImage(file);
  }
}

void ImageTexture::update(clock::time_point t) {
  _impl->update(t);
}

void ImageTexture::Impl::update(clock::time_point t) {

  ++animation_frame;

  if ((size_t)animation_frame > animation_range[1]) {

    if (do_loop) {

      GM_VINF("ImageTexture", "Looping animation");
      animation_frame = animation_range[0];

    } else if (do_exit) {

      exit(0);

    } else {

      GM_VINF("ImageTexture", "Animation done");
      animate = false;

    }
  }
}

bool ImageTexture::Impl::loadImage(std::string file_template, long int frame) {

  size_t filename_size = snprintf(nullptr, 0, file_template.c_str(), frame) + 1;
  std::vector<char> filename(filename_size + 1);
  snprintf(filename.data(), filename_size, file_template.c_str(), frame);

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

  GM_VINF("ImageTexture", "Loaded"
          << " image " << filename.data()
          << " " << image_width << "x" << image_height
          );
	return true;
}

void ImageTexture::update() {
  _impl->update();
}

GLuint ImageTexture::getGLTextureID() {
  return _impl->getGLTextureID();
}

END_NAMESPACE_GMGRAPHICS;

#endif
