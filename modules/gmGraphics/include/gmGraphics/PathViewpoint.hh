
#ifndef GRAMODS_GRAPHICS_PATHVIEWPOINT
#define GRAMODS_GRAPHICS_PATHVIEWPOINT

#include <gmGraphics/Viewpoint.hh>
#include <gmCore/Updateable.hh>

#include <gmCore/io_eigen.hh>

BEGIN_NAMESPACE_GMGRAPHICS;

/**
   The PathViewpoint animates the viewpoint position and
   orientation based on a specified path.
*/
class PathViewpoint
  : public gmGraphics::Viewpoint,
    public gmCore::Updateable {

public:

  PathViewpoint();

  /**
     Sets the path of the viewpoint.

     \gmXmlTag{gmGraphics,PathViewpoint,path}

     \sa gramods::operator>>(std::istream &, std::vector<Pose> &)
  */
  void setPath(std::vector<Pose> path);

  /**
     Adds a single node to the path of the viewpoint.

     \gmXmlTag{gmGraphics,PathViewpoint,node}

     \sa gramods::operator>>(std::istream &, Pose &)
  */
  void addNode(const Pose &node);

  /**
     Sets whether the viewpoint should loop over the path or not. Default is true.

     \gmXmlTag{gmGraphics,PathViewpoint,loop}
  */
  void setLoop(bool on);

  /**
     Activates or deactivates automatic exit when the path has been
     covered. Default is false.

     \gmXmlTag{gmGraphics,PathViewpoint,exit}
  */
  void setExit(bool on);

  /**
     Sets the velocity over the path, in meters per second. Default is 1 m/s.

     \gmXmlTag{gmGraphics,PathViewpoint,velocity}
  */
  void setVelocity(float v);

  /**
     Updates the animation.
  */
  void update(clock::time_point time, size_t frame) override;

  GM_OFI_DECLARE;

private:
  struct Impl;
  std::unique_ptr<Impl> _impl;

};

END_NAMESPACE_GMGRAPHICS;

#endif
