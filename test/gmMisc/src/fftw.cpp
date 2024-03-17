
#include <gmMisc/FFTW.hh>

#ifdef gramods_ENABLE_fftw

#include <gmCore/InvalidArgument.hh>
#include <gmCore/MathConstants.hh>

#include <fstream>

using namespace gramods;

#define COUNT 1024

TEST(gmMiscFFTW, Simple) {

  auto in = std::make_shared<gmMisc::FFTW::RealData>(COUNT);
  auto outE = std::make_shared<gmMisc::FFTW::ComplexData>(COUNT / 2);
  auto out = std::make_shared<gmMisc::FFTW::ComplexData>(COUNT / 2 + 1);

  EXPECT_THROW(gmMisc::FFTW fftwE(in, outE), gmCore::InvalidArgument);
  gmMisc::FFTW fftw(in, out);

  for (size_t idx = 0; idx < in->size; ++idx)
    in->data[idx] = std::sin(40.0 * GM_PI * idx / double(COUNT));

  fftw.execute();

  for (size_t idx = 0; idx < out->size; ++idx)
    if (idx != 20) EXPECT_LE(std::sqrt(std::norm(out->data[idx])), 1e-10);
  EXPECT_LT(std::sqrt(std::norm(out->data[20])) - COUNT / 2, 1e-10);
}

#endif
