/**
   (c)2013, 2017, 2018 Karljohan Lundin Palmerius
*/

#ifndef GRAMODS_MISC_EFFOAW
#define GRAMODS_MISC_EFFOAW

#include <gmMisc/config.hh>

#ifdef gramods_ENABLE_Eigen3

#include <deque>
#include <map>
#include <cstddef>
#include <limits>
#include <numeric>
#include <memory>

#include <math.h>

#include <Eigen/Eigen>

BEGIN_NAMESPACE_GMMISC

/**
   This is an end-fitting first-order adaptive window estimator of
   velocity from samples allowing jittering. Units of both length
   and time can be arbitrarily chosen and the velocity will be
   estimated as distance over time in the same units.

   The class uses std::valarray<double> to represent both position and
   velocity.
*/
class EFFOAW {

public:

  /** Creates a new end-fitting first-order adaptive window
      estimator using the default parameters. */
  EFFOAW();

  ~EFFOAW();

  /** Set how many samples should be saved to be used when
      estimating the velocity. */
  void setHistoryLength(size_t N);

  /** Get the number of samples that are saved to be used when
      estimating the velocity. */
  size_t getHistoryLength() const;

  /** Set how old samples should be saved to be used when
      estimating the velocity. */
  void setHistoryDuration(double t);

  /** Get how old samples are saved to be used when estimating the
      velocity. */
  double getHistoryDuration() const;

  /**
     Returns the time associated with the last sample associated
     with the specified id, or -1 if there is no last sample
     available.

     @param[in] id The id to read off the last sample for.

     @param[in] N Read off the Nth last sample, 0 (default) being the
     last sample.
  */
  double getLastSampleTime(size_t id, size_t N = 0);

  /**
     Add a position sample. Each sample is associated to an id and a
     time. If a new sample is provided with the same id and time as
     a previously added sample, this new sample will replace the
     old.

     @param[in] id The id to add position sample to.
     @param[in] position The position value to use as sample.
     @param[in] time The time at which the sample was taken.
  */
  void addSample(size_t id, Eigen::Vector3d position, double time);

  /**
     Remove the last sample for the specified id. This will remove the
     last sample what was actually added to the id; it will not
     restore a sample that was overwritten by another with the same
     time.

     @param[in] id The id to remove the last sample for.
  */
  void removeLastSample(size_t id);

  /**
     Estimate and return the velocity using samples associated to
     id. The velocity will be expressed in chosen units per second.

     @param[in] id The id to estimate velocity for.
     @param[in] error An estimate of the position tracking error, used
     to determine how many samples to include in the averaging.
     @param[out] samples The number of history samples used in the
     estimation.
  */
  Eigen::Vector3d estimateVelocity(size_t id, double error, size_t *samples = nullptr) const;

  /**
     Estimate and return the position using samples associated to
     id. This is not really a functionality of EFFOAW, but uses the
     EFFOAW estimated velocity as a means to estimate a position in
     the immediate future. It is assumed that the specified time
     occurs after or at the time of the last sample.

     @param[in] id The id to estimate position for.
     @param[in] error An estimate of the position tracking error, used
     to determine how many samples to include in the averaging.
     @param[in] time The time to estimate the position for.
     @param[out] samples The number of history samples used in the
     estimation.
  */
  Eigen::Vector3d estimatePosition(size_t id, double error, double time, size_t *samples = nullptr) const;

  /**
     Clean-up old samples and unused ids. This function should be
     called from time to time, for example after calling
     addSample, to avoid too much unnecessary memory usage.

     @param time The current time, to use when estimating if
     samples are too old, or -1 to use the samples' time.
  */
  void cleanup(double time = -1);

private:

  struct Impl;
  std::unique_ptr<Impl> _impl;

};

END_NAMESPACE_GMMISC

#endif
#endif
