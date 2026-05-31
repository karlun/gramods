
#ifndef GRAMODS_GRAPHICS_MIXINGSHADERS
#define GRAMODS_GRAPHICS_MIXINGSHADERS

#include <gmGraphics/config.hh>

#include <string>

BEGIN_NAMESPACE_GMGRAPHICS;

/**
   Shader code for mixing textures.

   This does nothing by itself but is instead used by MixingTexture
   and MixingView. See the documentation for those for details.

   Valid mix types are

   - average, showing the per pixel average value of all the sources

   - difference, showing the per pixel difference between the first
     and the second added source

   - distance, showing the per pixel color distance between the first
     and the second added source

   - alpha mask, showing the pixel color from second source with alpha
     from first source

   - red mask, showing the pixel color from second source, but using
     red from first source as alpha

   - alpha weighted average, showing the pixel average between second
     and third source, weighted by the alpha of the first source

   - red weighted average, showing the pixel average between second
     and third source, weighted by red of the first source

   @see MixingView
   @see MixingTexture
*/
struct MixingShaders {

  /**
     Checks if the specified mix_type is valid and returns true if it
     is. If it is not valid, then it logs a warning message with the
     specified tag before returning false.
  */
  static bool checkMixTypeValid(const std::string &mix_type,
                                const std::string &tag);

  /**
     Checks if the specified count of input textures is valid for the
     specified mix_type and returns true if it is. If it is not valid,
     then it logs a message with the specified tag before returning
     false if they are too few or true if they are too many.
  */
  static bool checkCount(const std::string &mix_type,
                         size_t count,
                         const std::string &tag,
                         const std::string &type);

  /**
     Returns fragment shader code for the specified mix_type.
  */
  static std::string getFragmentCode(const std::string &mix_type);
};

END_NAMESPACE_GMGRAPHICS;

#endif
