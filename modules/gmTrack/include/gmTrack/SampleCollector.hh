
#ifndef GM_TRACK_SAMPLECOLLECTOR
#define GM_TRACK_SAMPLECOLLECTOR

#include <gmTrack/config.hh>

#include <gmCore/io_eigen.hh>
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
  void update(clock::time_point time, size_t frame);

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
     Sets the data offset required to trigger a warning. Default is
     0.01, i.e. one cm.

     \gmXmlTag{gmTrack,SampleCollector,warningThreshold}
  */
  void setWarningThreshold(float d);

  /**
     Set the maximum positional distance from the average allowed for
     a sample to be included in the average. Default is -1 meaning
     that all samples are included.
  */
  void setInlierThreshold(float r);

  /**
     Sets the data offset required to trigger a warning. Default is
     π/4 radians.

     \gmXmlTag{gmTrack,SampleCollector,orientationWarningThreshold}
  */
  void setOrientationWarningThreshold(float d);

  /**
     Set the maximum orientational distance (in radians) from the
     average allowed for a sample to be included in the
     average. Default is -1 meaning that all samples are included.
  */
  void setOrientationInlierThreshold(float r);

  /**
     Returns the current list of tracker positions.
  */
  const std::vector<Eigen::Vector3f> &getTrackerPositions() const;

  /**
     Returns the current list of tracker orientations.
  */
  const std::vector<Eigen::Quaternionf> &getTrackerOrientations() const;

  /**
     Calculates and returns the average point of a set of
     samples. Optionally also standard deviation and maximum deviation
     can be estimated.

     @param[in] samples The samples to estimate average of
     @param[out] stddev Optional pointer to variable to write standard deviation to
     @param[out] maxdev Optional pointer to variable to write maximum deviation to
     @param[in] inlier_dist Optional threshold for counting as inlier
     @param[out] inlier_count Optional pointer to variable to write inlier count to
  */
  static Eigen::Vector3f getAverage(std::vector<Eigen::Vector3f> samples,
                                    float *stddev = nullptr,
                                    float *maxdev = nullptr,
                                    float inlier_dist = -1.f,
                                    size_t *inlier_count = nullptr);
  /**
     Calculates and returns the average point of a set of
     samples. Optionally also standard deviation and maximum deviation
     can be estimated (expressed in radians).

     @param[in] samples The samples to estimate average of
     @param[out] stddev Optional pointer to variable to write standard deviation to
     @param[out] maxdev Optional pointer to variable to write maximum deviation to
     @param[in] inlier_dist Optional threshold (in radians) for counting as inlier
     @param[out] inlier_count Optional pointer to variable to write inlier count to
  */
  static Eigen::Quaternionf getAverage(std::vector<Eigen::Quaternionf> samples,
                                       float *stddev = nullptr,
                                       float *maxdev = nullptr,
                                       float inlier_dist = -1.f,
                                       size_t *inlier_count = nullptr);

  /**
     Propagates the specified visitor.

     @see Object::Visitor
  */
  void traverse(Visitor *visitor) override;

  GM_OFI_DECLARE;

};

END_NAMESPACE_GMTRACK;

#endif
