/**
   (c)2024 Karljohan Lundin Palmerius
*/

#include <gmMisc/FFTW.hh>

#ifdef gramods_ENABLE_fftw

#include <gmCore/InvalidArgument.hh>
#include <gmCore/PreConditionViolation.hh>

#include <fftw3.h>

BEGIN_NAMESPACE_GMMISC

struct FFTW::Impl {
  Impl(std::shared_ptr<RealData> in, std::shared_ptr<ComplexData> out);
  ~Impl();

  fftw_plan plan;
  std::shared_ptr<RealData> in;
  std::shared_ptr<ComplexData> out;
};

FFTW::RealData::RealData(size_t N) : size(N), data(fftw_alloc_real(N)) {}
FFTW::RealData::~RealData() { if (data) fftw_free(data); }

FFTW::ComplexData::ComplexData(size_t N)
  : size(N),
    data(reinterpret_cast<std::complex<double> *>(fftw_alloc_complex(N))) {}
FFTW::ComplexData::~ComplexData() { if (data) fftw_free(data); }

FFTW::FFTW(std::shared_ptr<RealData> in, std::shared_ptr<ComplexData> out)
  : _impl(std::make_unique<Impl>(in, out)) {}
FFTW::~FFTW() {}

FFTW::Impl::Impl(std::shared_ptr<RealData> in, std::shared_ptr<ComplexData> out)
  : in(in), out(out) {

  if (in->size / 2 + 1 != out->size)
    throw gmCore::InvalidArgument(
        "Output size must be N/2+1 (where N is the input size)");

  plan = fftw_plan_dft_r2c_1d(
      in->size, in->data, reinterpret_cast<fftw_complex *>(out->data), 0);
}

FFTW::Impl::~Impl() {
  if (plan) fftw_destroy_plan(plan);
}

void FFTW::execute() {
  if (!_impl->plan)
    throw gmCore::PreConditionViolation("No valid plan available");
  fftw_execute(_impl->plan);
}

END_NAMESPACE_GMMISC

#endif
