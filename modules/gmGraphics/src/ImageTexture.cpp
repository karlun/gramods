
#include <gmGraphics/ImageTexture.hh>

#ifdef gramods_ENABLE_FreeImage

BEGIN_NAMESPACE_GMGRAPHICS;

namespace {
  struct _This {

    _This();

    GLuint getGLTextureID() { return texture_id; }

    GLuint texture_id = 0;
    std::shared_ptr<FreeImageInitializer> free_image_initializer;
  };
}

ImageTexture::ImageTexture()
  : _this(new _This) {}

void ImageTexture::initialize() {
  Texture::initialize();
}

_This::_This()
  : free_image_initializer(FreeImageInitializer::get()) {}

bool _This::loadImage(std::string filename) {

	FREE_IMAGE_FORMAT format = FreeImage_GetFileType(filename, 0);
	if(format == FIF_UNKNOWN) 
		format = FreeImage_GetFIFFromFilename(filename);
	if(format == FIF_UNKNOWN)
		return false;

	FIBITMAP *image = nullptr;
	if(FreeImage_FIFSupportsReading(format))
		image = FreeImage_Load(format, filename);
	//if the image failed to load, return failure
	if(!image)
		return false;

	BYTE* image_data = FreeImage_GetBits(image);
	unsigned int image_width = FreeImage_GetWidth(image);
	unsigned int image_height = FreeImage_GetHeight(image);
	if((image_data == 0) || (image_width == 0) || (image_height == 0))
		return false;
	
	//generate an OpenGL texture ID for this texture
	glGenTextures(1, &gl_texID);
	//store the texture ID mapping
	m_texID[texID] = gl_texID;
	//bind to the new texture ID
	glBindTexture(GL_TEXTURE_2D, gl_texID);
	//store the texture data for OpenGL use
	glTexImage2D(GL_TEXTURE_2D, level, internal_format, width, height,
		border, image_format, GL_UNSIGNED_BYTE, bits);

	//Free FreeImage's copy of the data
	FreeImage_Unload(image);

	//return success
	return true;
}

void ImageTexture::update() {}

GLuint getGLTextureID() {
  return _this->getGLTextureID();
}

END_NAMESPACE_GMGRAPHICS;

#endif
#endif
