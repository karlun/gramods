
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

  std::vector<double> getValue(const std::vector<double> &inval) const;
  Eigen::MatrixXd getJacobian(const std::vector<double> &in_values) const;
  double getDerivative(double inval) const;

  void clear();

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

PolyFit::PolyFit(const std::vector<std::vector<double>> &invals,
                 const std::vector<std::vector<double>> &outvals,
                 size_t ord)
  : PolyFit(invals.back().size(), outvals.back().size(), ord) {
  if (invals.size() != outvals.size())
    throw gmCore::InvalidArgument(GM_STR(
        "Invalid data sizes: " << invals.size() << " != " << outvals.size()));
  for (size_t idx = 0; idx < invals.size(); ++idx)
    addSample(invals[idx], outvals[idx]);
}

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

PolyFit::polco PolyFit::estimateCoefficients() const {
  return const_cast<PolyFit*>(this)->_impl->estimateCoefficients();
}

PolyFit::polco PolyFit::Impl::estimateCoefficients() {

  if (coeffs) return *coeffs;

  Eigen::MatrixXd poly(in_values.size(), COE);

  for (size_t sample_idx = 0; sample_idx < in_values.size(); ++sample_idx) {

    // Create list [ [ 1, x, x², ... ], [ 1, y, y², ...], ... ]
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

    // Create list [ 1, x, x², ..., yx, yx², ..., y²x, y²x², ... ]
    // by expanding [ 1, x, x², ... ] × [ 1, y, y², ... ] × ...
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

  auto XtX = poly.transpose() * poly; // NxM MxN -> NxN (order + 1)²
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

double PolyFit::getSingle(double inval) const {
  if (_impl->IDIM != 1 || _impl->ODIM != 1)
    throw gmCore::InvalidArgument(
        GM_STR("Cannot use 1D method on polynomial with dimensions "
               << _impl->IDIM << " and " << _impl->ODIM << "."));
  return getValue(std::vector<double> {inval})[0];
}

std::vector<double>
PolyFit::getValue(const std::vector<double> &in_values) const {
  return _impl->getValue(in_values);
}

double PolyFit::getDerivative(double in_value) const {
  if (_impl->IDIM != 1 || _impl->ODIM != 1)
    throw gmCore::InvalidArgument(
        GM_STR("Cannot use 1D method on polynomial with dimensions "
               << _impl->IDIM << " and " << _impl->ODIM << "."));
  return _impl->getDerivative(in_value);
}

std::vector<double>
PolyFit::Impl::getValue(const std::vector<double> &in_values) const {

  if (in_values.size() != IDIM)
    throw gmCore::InvalidArgument(GM_STR("Wrong size on polynomial input ("
                                         << in_values.size() << " != " << IDIM
                                         << ")."));

  const_cast<Impl *>(this)->estimateCoefficients();

  // Create list [ [ 1, x, x², ... ], [ 1, y, y², ...], ... ]
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

  // Create list [ 1, x, x², ..., yx, yx², ..., y²x, y²x², ... ]
  // by expanding [ 1, x, x², ... ] × [ 1, y, y², ... ] × ...
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

Eigen::MatrixXd
PolyFit::getJacobian(const std::vector<double> &in_values) const {
  return _impl->getJacobian(in_values);
}

Eigen::MatrixXd
PolyFit::Impl::getJacobian(const std::vector<double> &in_values) const {

  if (in_values.size() != IDIM)
    throw gmCore::InvalidArgument(GM_STR("Wrong size on polynomial input ("
                                         << in_values.size() << " != " << IDIM
                                         << ")."));

  const_cast<Impl *>(this)->estimateCoefficients();

  // Create list [ [ 1, x, x², ... ], [ 1, y, y², ...], ... ]
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

  Eigen::MatrixXd res(ODIM, IDIM);
  for (size_t odim = 0; odim < ODIM; ++odim) {
    for (size_t d_idim = 0; d_idim < IDIM; ++d_idim) {

      // Expanding partial derivatives,
      // e.g. [ 1, x, x², ... ] × [ 0, 1, y, ... ] × ...
      std::vector<double> comp_val;
      comp_val.push_back(1.0);
      for (size_t idim = 0; idim < IDIM; ++idim) {
        std::vector<double> res;
        res.reserve(comp_val.size() * (ORD + 1));
        for (size_t ord = 0; ord < ORD + 1; ++ord) {
          const auto c = idim != d_idim ? comp[idim][ord]
                         : ord > 0      ? ord * comp[idim][ord - 1]
                                        : 0.0;
          for (auto p : comp_val) res.push_back(c * p);
        }
        res.swap(comp_val);
      }

      for (size_t idx = 0; idx < COE; ++idx)
        res(odim, d_idim) += comp_val[idx] * (*coeffs)(odim, idx);
    }
  }

  return res;
}

double PolyFit::Impl::getDerivative(double in_value) const {

  const_cast<Impl *>(this)->estimateCoefficients();

  std::vector<double> comp;
  comp.reserve(ORD);

  double V = in_value;
  double T = 1;
  comp.push_back(T);
  for (size_t idx = 0; idx < ORD - 1; ++idx) {
    T *= V;
    comp.push_back(T);
  }

  double res = 0.0;
  for (size_t idx = 0; idx < COE - 1; ++idx)
    res += (idx + 1) * comp[idx] * (*coeffs)(0, idx + 1);

  return res;
}

void PolyFit::clear() { _impl->clear(); }

void PolyFit::Impl::clear() {
  in_values.clear();
  out_values.clear();
  coeffs = std::nullopt;
}

END_NAMESPACE_GMMISC;

#endif
