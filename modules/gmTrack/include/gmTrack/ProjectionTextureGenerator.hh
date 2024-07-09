
#ifndef GM_TRACK_PROJECTIONTEXTUREGENERATOR
#define GM_TRACK_PROJECTIONTEXTUREGENERATOR

#include <gmTrack/config.hh>

#ifdef gramods_ENABLE_FreeImage

// Required before gmCore/OFactory.hh for some compilers
#include <gmCore/io_size.hh>

#include <gmTrack/SampleCollector.hh>

#include <filesystem>

BEGIN_NAMESPACE_GMTRACK;

/**
   The ProjectionTextureGenerator is a utility for generating a
   projection texture for gmGraphics::TextureProjectedView.

   The utility is used by marking, with the tracking device, the
   position of known, pre-specified positions in the room or display
   system.
*/
class ProjectionTextureGenerator
  : public SampleCollector {

public:

  ProjectionTextureGenerator();
  ~ProjectionTextureGenerator();

  /**
     Set the resolution of the output image.
  */
  void setResolution(gmCore::size2 res);

  /**
     Creates a new region for the following calls to
     addBufferPosition, and optionally also sets its polynomial
     order. Use the regions to separate between geometrically smooth
     projection areas with discontinuities in between. Typically
     projection surfaces are flat (order = 1) or spherical/cylindrical
     (order = 2). If positions are added without specifying polynomial
     order, then order 1 will be used.

     @param order The polynomial order of the region.

     \gmXmlTag{gmTrack,ProjectionTextureGenerator,region}
  */
  void addRegion(size_t order = 1);

  /**
     Adds a known calibration point, in room coordinates. If positions
     are added without first specifying polynomial order (see
     addRegion), then order 1 will be used.

     \gmXmlTag{gmTrack,ProjectionTextureGenerator,bufferPosition}
  */
  void addBufferPosition(Eigen::Vector2f p);

  /**
     Adds a point, referenced by index, from the buffer position list
     to the list of hull points for the current region. A list of hull
     points that enclose the region is necessary when more than one
     region is defined. This starts at zero for each region.

     \gmXmlTag{gmTrack,ProjectionTextureGenerator,hullPoint}
  */
  void addHullIndex(size_t idx);

  /**
     Adds a point to the list of hull points for the current region. A
     list of hull points that enclose the region is necessary when
     more than one region is defined.

     \gmXmlTag{gmTrack,ProjectionTextureGenerator,hullPoint}
  */
  void addHullPosition(Eigen::Vector2f p);

  /**
     Sets the file path to save the view to. This generator will save
     the image in TIFF format (32 bit float RGB encoding XYZ) so if
     the specified path does not include a .tif suffix then one will
     be added.

     \gmXmlTag{gmTrack,ProjectionTextureGenerator,file}
  */
  void setFile(std::filesystem::path file);

  /**
     Save the final image to the specified file. Since the RGB values
     of the image will be in the range 0-1, a scale and offset for
     these values will also be computed.
  */
  void saveImage();

  /**
     Returns the offset needed to convert the image pixel values in the
     last generated image to their respective 3D position.
  */
  Eigen::Vector3f getOffset();

  /**
     Returns the scale needed to convert the image pixel values in the
     last generated image to their respective 3D position.
  */
  Eigen::Vector3f getScale();

  GM_OFI_DECLARE;

private:
  struct Impl;
};

END_NAMESPACE_GMTRACK;

#endif
#endif
