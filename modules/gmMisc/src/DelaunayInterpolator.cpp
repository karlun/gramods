
#include <gmMisc/DelaunayInterpolator.hh>

#if defined(gramods_ENABLE_Eigen3) && defined(gramods_ENABLE_Lehdari_Delaunay)

#include <gmCore/InvalidArgument.hh>
#include <gmCore/Stringify.hh>
#include <gmCore/ExitException.hh>

#include <Lehdari/Delaunay.hpp>
#include <memory>
#include <iostream>

BEGIN_NAMESPACE_GMMISC;

struct DelaunayInterpolator::Impl {

  Impl(size_t idim, size_t odim) : IDIM(idim), ODIM(odim) {
    if (idim != 2)
      throw gmCore::InvalidArgument(
          GM_STR("Incorrect input dimensions: " << idim << " (expected 2)."));
  }

  void addSample(const std::vector<double> &inval,
                 const std::vector<double> &outval);
  std::vector<double> getValue(const std::vector<double> &inval);

  std::vector<
      std::pair<Eigen::Matrix<double, 2, 1>, Eigen::Matrix<double, 2, 1>>>
      values;
  std::vector<int32_t> indices;
  size_t last_hit_idx = 0;

  const size_t IDIM;
  const size_t ODIM;
};

DelaunayInterpolator::DelaunayInterpolator(size_t idim, size_t odim)
  : _impl(std::make_unique<Impl>(idim, odim)) {}

DelaunayInterpolator::~DelaunayInterpolator() {}

void DelaunayInterpolator::addSample(const std::vector<double> &inval,
                                     const std::vector<double> &outval) {
  _impl->addSample(inval, outval);
}

void DelaunayInterpolator::Impl::addSample(const std::vector<double> &inval,
                                           const std::vector<double> &outval) {

  if (inval.size() != IDIM)
    throw gmCore::InvalidArgument(
        GM_STR("Incorrect input dimensions: " << inval.size() << " (expected "
                                              << IDIM << ")."));

  if (outval.size() != ODIM)
    throw gmCore::InvalidArgument(
        GM_STR("Incorrect output dimensions: " << outval.size() << " (expected "
                                               << ODIM << ")."));

  indices.clear();
  values.emplace_back(Eigen::Matrix<double, 2, 1>(inval[0], inval[1]),
                      Eigen::Matrix<double, 2, 1>(outval[0], outval[1]));
  //out_values.push_back(
  //Eigen::Map<const Eigen::MatrixXd>(outval.data(), outval.size(), 1));
}

std::vector<double>
DelaunayInterpolator::getValue(const std::vector<double> &inval) {
  return _impl->getValue(inval);
}

std::vector<double>
DelaunayInterpolator::Impl::getValue(const std::vector<double> &inval) {

  Eigen::Matrix<double, 2, 1> pt(inval[0], inval[1]);

  if (indices.empty()) {

    // Need in and out values sorted otherwise triangulate does that
    // for in_values, leaving values misaligned with indices
    std::sort(values.begin(),
              values.end(),
              [](const std::pair<Eigen::Matrix<double, 2, 1>,
                                 Eigen::Matrix<double, 2, 1>> &A,
                 const std::pair<Eigen::Matrix<double, 2, 1>,
                                 Eigen::Matrix<double, 2, 1>> &B) {
                return A.first(0, 0) < B.first(0, 0) ||
                       (A.first(0, 0) == B.first(0, 0) &&
                        A.first(1, 0) < B.first(1, 0));
              });

    std::vector<Eigen::Matrix<double, 2, 1>> in_values;
    in_values.reserve(values.size());
    for (auto & v : values) in_values.push_back(v.first);

    indices = delaunay::triangulate(in_values);

    last_hit_idx = 0;
  }

  for (size_t pidx = 0; pidx < indices.size(); pidx += 3) {

    size_t idx = (last_hit_idx + pidx) % indices.size();

    if (indices[idx + 2] < 0) continue;

    auto &pt0 = values[indices[idx + 0]].first;
    auto &pt1 = values[indices[idx + 1]].first;
    auto &pt2 = values[indices[idx + 2]].first;
    if (pt(0, 0) < pt0(0, 0) && pt(0, 0) < pt1(0, 0) && pt(0, 0) < pt2(0, 0))
      continue;
    if (pt(1, 0) < pt0(1, 0) && pt(1, 0) < pt1(1, 0) && pt(1, 0) < pt2(1, 0))
      continue;
    if (pt0(0, 0) < pt(0, 0) && pt1(0, 0) < pt(0, 0) && pt2(0, 0) < pt(0, 0))
      continue;
    if (pt0(1, 0) < pt(1, 0) && pt1(1, 0) < pt(1, 0) && pt2(1, 0) < pt(1, 0))
      continue;

    auto v0 = pt1 - pt0;
    auto v1 = pt2 - pt0;
    auto v2 = pt - pt0;
    auto d00 = v0.dot(v0);
    auto d01 = v0.dot(v1);
    auto d11 = v1.dot(v1);
    auto d20 = v2.dot(v0);
    auto d21 = v2.dot(v1);
    auto denom = d00 * d11 - d01 * d01;

    if (denom < std::numeric_limits<double>::epsilon()) continue;

    auto v = (d11 * d20 - d01 * d21) / denom;
    if (v < 0 || 1.0 < v) continue;

    auto w = (d00 * d21 - d01 * d20) / denom;
    if (w < 0 || 1.0 < w) continue;

    auto u = 1.0 - v - w;
    if (u < 0 || 1.0 < u) continue;

    last_hit_idx = idx;

    auto res = u * values[indices[idx + 0]].second +
               v * values[indices[idx + 1]].second +
               w * values[indices[idx + 2]].second;

    std::vector<double> ret;
    ret.reserve(ODIM);
    for (size_t dim = 0; dim < ODIM; ++dim) ret.push_back(res(dim, 0));

    return ret;
  }

  return {};
}

END_NAMESPACE_GMMISC;

#endif
