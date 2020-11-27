
#ifndef GM_TRACK_SAMPLECOLLECTOR
#define GM_TRACK_SAMPLECOLLECTOR

#include <gmTrack/config.hh>

#include <gmCore/eigen.hh>
#include <gmCore/Object.hh>
#include <gmTrack/Controller.hh>
#include <gmCore/Updateable.hh>

BEGIN_NAMESPACE_GMTRACK;

/**
   The SampleCollector is a utility for sampling tracker data.
*/
class SampleCollector
  : public gmCore::Object,
    public gmCore::Updateable {

protected:
  struct Impl;
  std::unique_ptr<Impl> _impl;

public:
  SampleCollector(Impl *impl = nullptr);
  ~SampleCollector();

  /**
     Updates the internal states. Automatically called via
     gmCore::Updateable::updateAll().
  */
  void update(clock::time_point t);

  /**
     Sets the controller to use for reading tracker samples.

     \gmXmlTag{gmTrack,SampleCollector,controller}
  */
  void setController(std::shared_ptr<Controller> controller);

  /**
     Adds a tracker position.

     \gmXmlTag{gmTrack,SampleCollector,trackerPosition}
  */
  void addTrackerPosition(Eigen::Vector3f p);

  /**
     Adds a tracker orientation.

     \gmXmlTag{gmTrack,SampleCollector,trackerOrientation}
  */
  void addTrackerOrientation(Eigen::Quaternionf o);

  /**
     Sets the frequency at which samples are collected when the
     controller button is pressed. Default is 1. At most one sample
     per frame will be used regardless of this value.

     \gmXmlTag{gmTrack,SampleCollector,samplesPerSecond}
  */
  void setSamplesPerSecond(float n);

  /**
     Returns the current list of tracker positions.
  */
  const std::vector<Eigen::Vector3f> &getTrackerPositions() const;

  /**
     Returns the current list of tracker orientations.
  */
  const std::vector<Eigen::Quaternionf> &getTrackerOrientations() const;

  GM_OFI_DECLARE;

};

END_NAMESPACE_GMTRACK;

#endif
