
#ifndef GRAMODS_GRAPHICS_TEXTUREINTERFACE
#define GRAMODS_GRAPHICS_TEXTUREINTERFACE

#include <gmGraphics/config.hh>

#include <gmGraphics/Eye.hh>

#include <GL/glew.h>
#include <GL/gl.h>

#include <optional>

BEGIN_NAMESPACE_GMGRAPHICS;

/**
   The interface for classes providing texture data for rendering.
*/
class TextureInterface {

public:
  /**
     Type of color in the texture.
  */
  enum TextureColor { GRAY, RGB, BGR };

  /**
     Returns the GLSL swizzling required to get RGB from the specified
     color type.
   */
  static std::string getRgbSwizzle(TextureColor c);

  /**
     Returns the GLSL swizzling required to get RGBA from the
     specified color type.
   */
  static std::string getRgbaSwizzle(TextureColor c);

  /**
     Information associated with the produced texture.
  */
  struct TextureData {
    /**
       OpenGL texture ID.
    */
    GLuint id;

    /**
       Color data in the texture.
    */
    TextureColor color;

    /**
       The frame number when the texture was last updated.
    */
    size_t frame_number;
  };

  /**
     Updates the texture and returns the ID of the associated GL
     texture object. Must be called with GL context. Observe also that
     this method may perform off-screen rendering.

     @param frame_number The current frame being rendered. This number
     should increment by one for each frame, however it may wrap
     around and start over at zero.

     @param eye Eye for which the texture is requested. An
     implementation may use different textures for different eyes or
     may reuse the same texture for all eyes.

     @returns Data about the texture or nullopt if the texture was
     invalid.
  */
  virtual std::optional<TextureData> updateTexture(size_t frame_number,
                                                   Eye eye) = 0;
};

END_NAMESPACE_GMGRAPHICS;

#endif
