/**
   (c)2022 Karljohan Lundin Palmerius
*/

#ifndef GRAMODS_MISC_POLYFIT
#define GRAMODS_MISC_POLYFIT

#include <gmMisc/config.hh>

#ifdef gramods_ENABLE_Eigen3

#include <Eigen/Eigen>
#include <memory>

BEGIN_NAMESPACE_GMMISC

/**
   This is a polygonal fitter for any number of in- and output
   dimensions allowing for inter- and extrapolation.
*/
class PolyFit {

public:
  /** Matrix holding the polynomial coefficients, one row for each
      output dimension. */
  typedef Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic> polco;

  /**
     Creates a PolyFit instance for the specified number of input and
     output dimensions, and polynomial order.

     @param[in] idim Input dimensionality.

     @param[in] odim Output dimensionality.

     @param[in] ord Order of the polynomial.
  */
  PolyFit(size_t idim, size_t odim, size_t ord);

  /**
     Creates a PolyFit instance and adds the specified sample values.
  */
  PolyFit(const std::vector<std::vector<double>> &invals,
          const std::vector<std::vector<double>> &outvals,
          size_t ord);

  ~PolyFit();

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
     Estimate and return the coefficients of the polynomial that best
     fits the data.

     This method will cache the coefficients until new samples have
     been added.
  */
  polco estimateCoefficients() const;

  /**
     Estimates the polynomial result from the specified input.

     This method calls estimateCoefficients (if necessary) and
     calculates the resulting polynomial with the specified
     input. This method can be used both for inter- and extrapolation.

     The dimensions of the input must match the dimensionality of the
     object or exception will be thrown.
  */
  std::vector<double> getValue(const std::vector<double> &inval) const;

  /**
     Convenience method for 1D polynomials.
     \sa getValue(const std::vector<double>&)
  */
  double getValue(double inval) const;

  /**
     Resets the contents of this object.
  */
  void clear();

private:

  struct Impl;
  std::unique_ptr<Impl> _impl;

};

END_NAMESPACE_GMMISC

#endif
#endif
