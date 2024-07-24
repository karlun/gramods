
#ifndef GRAMODS_GRAPHICS_OBJRENDERER
#define GRAMODS_GRAPHICS_OBJRENDERER

#include <gmGraphics/Renderer.hh>
#include <gmGraphics/ImageTexture.hh>

#include <gmCore/io_eigen.hh>
#include <gmCore/OFactory.hh>

BEGIN_NAMESPACE_GMGRAPHICS;

/**
   A renderer that draws an obj file.
*/
class ObjRenderer
  : public Renderer {

public:
  /**
     Data struct for material data used in the obj file rendering. The
     texture color value for each material component (texture_X) is
     multiplied with the corresponding color vector (color_X) before
     adding its contribution to the raster color.
  */
  struct Material {

    // Ambient color component. Default (0.0, 0.0, 0.0)
    Eigen::Vector3f color_ambient;
    // Diffuse color component. Default (0.8, 0.8, 0.8)
    Eigen::Vector3f color_diffuse;
    // Specular color component. Default (0.0, 0.0, 0.0)
    Eigen::Vector3f color_specular;
    // Emissive color component. Default (0.0, 0.0, 0.0)
    Eigen::Vector3f color_emissive;

    /**
       Shininess, i.e. the exponent of the specular
       highlights. Default 32.
    */
    float shininess;

    // Ambient texture
    std::shared_ptr<gmGraphics::ImageTexture> texture_ambient = 0;
    // Diffuse texture
    std::shared_ptr<gmGraphics::ImageTexture> texture_diffuse = 0;
    // Specular texture
    std::shared_ptr<gmGraphics::ImageTexture> texture_specular = 0;
    // Emissive texture
    std::shared_ptr<gmGraphics::ImageTexture> texture_emissive = 0;
  };

  ObjRenderer();
  ~ObjRenderer();

  /**
     Called to initialize the Object. This should be called once only!
  */
  void initialize() override;

  /**
     Performs rendering of 3D objects in the scene.
  */
  void render(const Camera &camera, const Eigen::Affine3f &Mm) override;

  /**
     Extracts the currently optimal near and far plane distances. This
     is typically used by a View if there are multiple renderers that
     need to be rendered with the same near and far planes for correct
     depth testing.
  */
  void getNearFar(const Camera &camera,
                  const Eigen::Affine3f &Mm,
                  float &near,
                  float &far) override;

  /**
     Set the obj file to render.

     \gmXmlTag{gmGraphics,ObjRenderer,file}
  */
  void setFile(std::filesystem::path file);

  /**
     Set to true if the model should be recentered after
     loading. Default is false.

     \gmXmlTag{gmGraphics,ObjRenderer,recenter}
  */
  void setRecenter(bool on);

  /**
     Returns the currently loaded materials.
  */
  std::vector<Material> getMaterials() const;

  /**
     Sets the materials to use. The specified list must have the same
     length as the loaded material list.
  */
  void setMaterials(const std::vector<Material> &materials);

  /**
     Check and return intersections between the provided line and the
     shape represented by this node.
  */
  std::vector<float> getIntersections(const IntersectionLine &line) override;

  GM_OFI_DECLARE;

private:

  struct Impl;
  std::unique_ptr<Impl> _impl;

};

END_NAMESPACE_GMGRAPHICS;

#endif
