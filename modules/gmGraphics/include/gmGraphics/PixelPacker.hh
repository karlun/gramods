
#ifndef GRAMODS_GRAPHICS_PIXELPACKER
#define GRAMODS_GRAPHICS_PIXELPACKER

#include <gmGraphics/config.hh>

#include <GL/glew.h>
#include <GL/gl.h>

#include <atomic>
#include <functional>
#include <memory>

BEGIN_NAMESPACE_GMGRAPHICS;

/**
   The PixelPacker is a utility for managing pixel packing procedures
   using one or more PixelBufferObject (PBO).
*/
class PixelPacker {

public:
  PixelPacker();
  virtual ~PixelPacker();

  /**
     Data shared with the consumer of the pixel data, i.e. the pixel
     data themselves and a flag for thread synchronization.
  */
  struct DataTransfer {

    /// The extracked (packed) pixel data
    GLubyte *data = nullptr;

    /**
       Flag for thread synchronization. Set this to true before
       handing over the data to another thread and set it back to
       false when that thread is done with the data.
    */
    std::atomic<bool> is_in_use = false;
  };

  /**
     The callback function called from execute with the mapped
     data. If the byte array is used in another thread, then set
     is_in_use to true before returning the function and set it back
     to false when done with the data.
  */
  typedef std::function<void(std::shared_ptr<DataTransfer>)> callback_function;

  /**
     Initializes the necessary objects.
  */
  bool init(size_t count);

  /**
     Bind a PBO with the specified byte count and associate its data
     with the specified callback function. The callback function will
     be called when doMap is called.
  */
  void bind(size_t byte_count, callback_function ufunc);

  /**
     Simply calls glBindBuffer(GL_PIXEL_PACK_BUFFER,0)
  */
  void unbind();

  /**
     Call this to use the downloaded data. This will map the buffer
     data and call the callback function. Calling this too many times
     has no effect.
  */
  void execute();

  /**
     Call this to call execute() and unmap the data until all buffers
     have been released.
  */
  void executeAndUnmapAll();

private:
  struct Impl;
  std::unique_ptr<Impl> _impl;
};

END_NAMESPACE_GMGRAPHICS;

#endif
