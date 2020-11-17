
#include <gmCore/Updateable.hh>
#include <gmGraphics/OsgRenderer.hh>
#include <gmTrack/Controller.hh>
#include <gmNetwork/SyncNode.hh>

using namespace gramods;

/**
 * My simple OSG app.
 *
 * This demo applies the *pimpl* technique to separate external
 * interface, used to call the app from the system, and internal
 * functionality that then can fully reside in `MyApp.cpp`.
 */
class MyApp : public gmCore::Updateable {

public:
  /**
   * Called from main() to create the app.
   */
  MyApp(std::vector<std::shared_ptr<gmNetwork::SyncNode>> sync_nodes,
        std::vector<std::shared_ptr<gmTrack::Controller>> controllers,
        std::shared_ptr<gmTrack::SinglePoseTracker> head);

  /**
   * For pimpl to work with std::unique_ptr the destructor must be
   * implemented at a point in the code where the struct Impl is
   * complete.
   */
  ~MyApp();

  /**
   * From gmCore::Updateable. Automatically called from main() via
   * gmCore::Updateable::updateAll();
   */
  void update(clock::time_point t) override;

  /**
   * We set up our own internal gmGraphics::OsgRenderer to render the
   * OpenSceneGraph scenegraph. This is returned to main() so that it
   * can be added to the gmGraphics::Window.
   */
  std::shared_ptr<gmGraphics::OsgRenderer> getRenderer();

private:
  struct Impl;
  std::unique_ptr<Impl> _impl;
};
