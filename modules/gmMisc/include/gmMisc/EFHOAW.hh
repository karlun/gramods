/**
   (c)2013, 2017, 2018 Karljohan Lundin Palmerius
*/

#ifndef GRAMODS_MISC_EFHOAW
#define GRAMODS_MISC_EFHOAW

#include <gmMisc/config.hh>

#ifdef gramods_ENABLE_Eigen3

#include <Eigen/Eigen>

#include <memory>

BEGIN_NAMESPACE_GMMISC

/**
   This is an end-fitting higher-order adaptive window estimator of
   velocity from samples allowing jittering. Units of both length and
   time can be arbitrarily chosen and the velocity will be estimated
   as distance over time in the same units.
*/
class EFHOAW {

public:

  /** Matrix holding the polynomial coefficients, one row for each
      dimension. */
  typedef Eigen::Matrix<double, 3, Eigen::Dynamic> polco;

  /** Creates a new end-fitting first-order adaptive window
      estimator using the default parameters. */
  EFHOAW();

  ~EFHOAW();

  /**
     Set how many samples should be saved to be used when estimating
     the velocity. Per default all samples are saved and used.
  */
  void setHistoryLength(size_t N);

  /** Get the number of samples that are saved to be used when
      estimating the velocity. */
  size_t getHistoryLength() const;

  /**
     Set how old samples should be saved to be used when estimating
      the velocity. Per default all samples are saved and used.
  */
  void setHistoryDuration(double t);

  /** Get how old samples are saved to be used when estimating the
      velocity. */
  double getHistoryDuration() const;

  /**
     Returns the time associated with the last sample associated
     with the specified id, or -1 if there is no last sample
     available.
  */
  double getLastSampleTime(size_t id);

  /**
     Add a position sample. Each sample is associated to an id and a
     time. If a new sample is provided with the same id and time as
     a previously added sample, this new sample will replace the
     old.

     @param[in] id The id to add position sample to.

     @param[in] position The position value to use as sample.

     @param[in] time The time at which the sample was taken.

     @param[in] replace If true (default), old samples with the same
     time will be replaced with this new sample value.
  */
  void addSample(size_t id, Eigen::Vector3d position, double time, bool replace = true);

  /**
     Estimate and return the coefficients of the polynomial, of the
     specified order, that best fits the data. This method uses as
     many samples backwards that gives an estimate within the
     specified error.

     @param[in] id The id to estimate velocity for.
     @param[in] error An estimate of the position tracking error, used
     to determine how many samples to include in the averaging.
     @param[in] order The order of estimation
     @param[out] samples The number of history samples used in the
     estimation.

     @return A 3 rows and order+1 column matrix with the polynomial
     coefficients
  */
  polco estimateCoefficients
  (size_t id, double error, size_t order = 2, size_t *samples = nullptr);

  /**
     Returns the position calculated from the polynomial using the
     last estimated coefficients, from the previous call to
     estimateCoefficients. Observe that this method can be used both
     for inter- and extrapolation.
  */
  Eigen::Vector3d getPolynomialPosition(int id, double t) const;

  /**
     Returns the velocity calculated from the polynomial using the
     last estimated coefficients, from the previous call to
     estimateCoefficients. Observe that this method can be used both
     for inter- and extrapolation.
  */
  Eigen::Vector3d getPolynomialVelocity(int id, double t) const;

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
