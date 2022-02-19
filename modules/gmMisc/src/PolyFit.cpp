
#include <gmMisc/PolyFit.hh>

#ifdef gramods_ENABLE_Eigen3

#include <gmCore/InvalidArgument.hh>
#include <gmCore/Stringify.hh>

#include <gmMisc/ConstexprPow.hh>

#include <iostream>
#include <optional>

BEGIN_NAMESPACE_GMMISC;

struct PolyFit::Impl {

  typedef PolyFit::polco polco;

  Impl(size_t idim, size_t odim, size_t ord);

  void addSample(const std::vector<double> &inval,
                 const std::vector<double> &outval);

  polco estimateCoefficients();

  std::vector<double>
  getPolynomialPosition(const std::vector<double> &inval);

  std::vector<std::vector<double>> in_values;
  std::vector<std::vector<double>> out_values;

  std::optional<polco> coeffs;

  const size_t IDIM;
  const size_t ODIM;
  const size_t ORD;
  const size_t COE;
};

PolyFit::PolyFit(size_t idim, size_t odim, size_t ord)
  : _impl(std::make_unique<Impl>(idim, odim, ord)) {}

PolyFit::~PolyFit() {}

PolyFit::Impl::Impl(size_t idim, size_t odim, size_t ord)
  : IDIM(idim), ODIM(odim), ORD(ord), COE(ipow(ORD + 1, IDIM)) {}

void PolyFit::addSample(const std::vector<double> &inval,
                        const std::vector<double> &outval) {
  _impl->addSample(inval, outval);
}

void PolyFit::Impl::addSample(const std::vector<double> &inval,
                              const std::vector<double> &outval) {

  if (inval.size() != IDIM)
    throw gmCore::InvalidArgument(
        GM_STR("Incorrect input dimensions: " << inval.size() << " (expected "
                                              << IDIM << ")."));

  if (outval.size() != ODIM)
    throw gmCore::InvalidArgument(
        GM_STR("Incorrect output dimensions: " << outval.size() << " (expected "
                                               << ODIM << ")."));

  coeffs = std::nullopt;
  in_values.push_back(inval);
  out_values.push_back(outval);
}

PolyFit::polco PolyFit::estimateCoefficients() {
  return _impl->estimateCoefficients();
}

PolyFit::polco PolyFit::Impl::estimateCoefficients() {

  if (coeffs) return *coeffs;

  Eigen::MatrixXd poly(in_values.size(), COE);

  for (size_t sample_idx = 0; sample_idx < in_values.size(); ++sample_idx) {

    // create list comp = [ [ 1, x, x^2 ], [ 1, y, y^2], ... ]
    std::vector<std::vector<double>> comp(IDIM);
    for (size_t idim = 0; idim < IDIM; ++idim) {
      std::vector<double> &vals = comp[idim];
      vals.reserve(ORD + 1);

      double V = in_values[sample_idx][idim];
      double T = 1;
      vals.push_back(T);
      for (size_t ord_idx = 0; ord_idx < ORD; ++ord_idx) {
        T *= V;
        vals.push_back(T);
      }
    }

    // create 1 * (1 + x + x^2 ) * (1 + y + y^2) ...
    std::vector<double> comp_val;
    comp_val.push_back(1.0);
    for (size_t idim = 0; idim < IDIM; ++idim) {
      std::vector<double> res;
      res.reserve(comp_val.size() * (ORD + 1));
      for (auto c : comp[idim])
        for (auto p : comp_val)
          res.push_back(c*p);
      res.swap(comp_val);
    }

    for (size_t comp_idx = 0; comp_idx < comp_val.size(); ++comp_idx)
      poly(sample_idx, comp_idx) = comp_val[comp_idx];
  }

  auto XtX = poly.transpose() * poly; // NxM MxN -> NxN (order + 1)^2
  auto XtXinv = XtX.inverse();
  auto XtXinvXt = XtXinv * poly.transpose();

  Eigen::MatrixXd res(ODIM, COE);
  for (size_t odim = 0; odim < ODIM; ++odim) {
    Eigen::MatrixXd vec(out_values.size(), 1);
    for (size_t sample_idx = 0; sample_idx < out_values.size(); ++sample_idx)
      vec(sample_idx, 0) = out_values[sample_idx][odim];
    res.row(odim) = (XtXinvXt * vec).transpose();
  }

  coeffs = res;
  return res;
}

std::vector<double>
PolyFit::getPolynomialPosition(const std::vector<double> &in_values) {
  return _impl->getPolynomialPosition(in_values);
}

std::vector<double>
PolyFit::Impl::getPolynomialPosition(const std::vector<double> &in_values) {

  if (in_values.size() != IDIM)
    throw gmCore::InvalidArgument(GM_STR("Wrong size on polynomial input ("
                                         << in_values.size() << " != " << IDIM
                                         << ")."));

  estimateCoefficients();

  std::vector<std::vector<double>> comp(IDIM);
  for (size_t idim = 0; idim < IDIM; ++idim) {
    std::vector<double> &vals = comp[idim];
    vals.reserve(ORD + 1);

    double V = in_values[idim];
    double T = 1;
    vals.push_back(T);
    for (size_t ord_idx = 0; ord_idx < ORD; ++ord_idx) {
      T *= V;
      vals.push_back(T);
    }
  }

  // create 1 * (1 + x + x^2 ) * (1 + y + y^2) ...
  std::vector<double> comp_val;
  comp_val.push_back(1.0);
  for (size_t idim = 0; idim < IDIM; ++idim) {
    std::vector<double> res;
    res.reserve(comp_val.size() * (ORD + 1));
    for (auto c : comp[idim])
      for (auto p : comp_val)
        res.push_back(c*p);
    res.swap(comp_val);
  }

  std::vector<double> res(ODIM, 0.0);
  for (size_t odim = 0; odim < ODIM; ++odim)
    for (size_t idx = 0; idx < COE; ++idx)
      res[odim] += comp_val[idx] * (*coeffs)(odim, idx);

  return res;
}

END_NAMESPACE_GMMISC;

#endif
