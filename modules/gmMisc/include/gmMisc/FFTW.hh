/**
   (c)2024 Karljohan Lundin Palmerius
*/

#ifndef GRAMODS_MISC_FFTW
#define GRAMODS_MISC_FFTW

#include <gmMisc/config.hh>

#ifdef gramods_ENABLE_fftw

#include <complex>
#include <memory>

BEGIN_NAMESPACE_GMMISC

/**
   This is FFT using FFTW as back-end.
*/
class FFTW {

public:

  struct RealData {
    RealData(size_t N);
    ~RealData();
    const size_t size;
    double *const data;
  };

  struct ComplexData {
    ComplexData(size_t N);
    ~ComplexData();
    const size_t size;
    std::complex<double> *const data;
  };

  FFTW(std::shared_ptr<RealData> in, std::shared_ptr<ComplexData> out);
  virtual ~FFTW();

  void execute();

private:
  struct Impl;
  std::unique_ptr<Impl> _impl;
};

END_NAMESPACE_GMMISC

#endif
#endif
