/**
   (c)2022 Karljohan Lundin Palmerius
*/

#ifndef GRAMODS_MISC_DELAUNAYINTERPOLATOR
#define GRAMODS_MISC_DELAUNAYINTERPOLATOR

#include <gmMisc/config.hh>

#if defined(gramods_ENABLE_Eigen3) && defined(gramods_ENABLE_Lehdari_Delaunay)

#include <memory>
#include <vector>

BEGIN_NAMESPACE_GMMISC

/**
   DelaunayInterpolator
*/
class DelaunayInterpolator {

public:
  /**
     Creates an instance for the specified number of input and output
     dimensions.

     Currently only 2D supported, i.e. idim = 2.
  */
  DelaunayInterpolator(size_t idim, size_t odim);
  ~DelaunayInterpolator();

  /**
     Add a sample specifying the relation between input and output
     values. The dimensions must match the dimensionality of the
     object or exception will be thrown.

     @param[in] inval The parameter value that should give rise to the
     specified output value.

     @param[in] outval The output value for the specified inval.
  */
  void addSample(const std::vector<double> &inval,
                 const std::vector<double> &outval);

  /**
     Estimates the interpolated value for the specified input.

     The dimensions of the input must match the dimensionality of the
     object or exception will be thrown.
  */
  std::vector<double> getValue(const std::vector<double> &inval);

private:

  struct Impl;
  std::unique_ptr<Impl> _impl;

};

END_NAMESPACE_GMMISC

#endif
#endif
