
#ifndef GRAMODS_GRAPHICS_GROUP
#define GRAMODS_GRAPHICS_GROUP

#include <gmGraphics/Node.hh>

BEGIN_NAMESPACE_GMGRAPHICS;

/**
   The scenegraph group base.
*/
class Group : public Node {

public:
  Group();
  virtual ~Group();

  /**
     Adds a child
  */
  void addNode(std::shared_ptr<Node> node);

  /**
     Propagates the specified visitor.

     @see Object::Visitor
  */
  void traverse(Visitor *visitor) override;

  GM_OFI_DECLARE;

private:
  struct Impl;
  std::unique_ptr<Impl> _impl;
};

END_NAMESPACE_GMGRAPHICS;

#endif
