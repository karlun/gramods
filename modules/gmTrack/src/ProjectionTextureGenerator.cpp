
#include <gmTrack/ProjectionTextureGenerator.hh>

#ifdef gramods_ENABLE_FreeImage

#include <gmTrack/SampleCollector.impl.hh>

#include <gmTrack/ButtonsMapper.hh>

#include <gmCore/RunOnce.hh>
#include <gmCore/Console.hh>
#include <gmCore/PreConditionViolation.hh>
#include <gmCore/io_float.hh>
#include <gmCore/MathConstants.hh>

#include <gmMisc/PolyFit.hh>

#include <gmCore/FreeImage.hh>
#include <FreeImage.h>

#include <limits>
//#include <type_traits>

BEGIN_NAMESPACE_GMTRACK;

GM_OFI_DEFINE_SUB(ProjectionTextureGenerator, SampleCollector);
GM_OFI_PARAM2(ProjectionTextureGenerator, bufferPosition, Eigen::Vector2f, addBufferPosition);
GM_OFI_PARAM2(ProjectionTextureGenerator, region, size_t, addRegion);

struct ProjectionTextureGenerator::Impl : SampleCollector::Impl {

  typedef gmCore::Updateable::clock clock;

  std::vector<size_t> region_order;
  std::vector<std::vector<Eigen::Vector2f>> region_positions;
  std::vector<std::vector<Eigen::Vector2f>> region_hull_positions;
  size_t total_region_positions_count = 0;

  std::filesystem::path file = "output.tif";
  gmCore::size2 resolution = {4096, 4096};

  Eigen::Vector3f scale = Eigen::Vector3f::Ones();
  Eigen::Vector3f offset = Eigen::Vector3f::Zero();

  void update(clock::time_point) override;
  void addRegion(size_t order);
  void saveImage();
  void checkPreconditions();
  size_t getRegionIdx(Eigen::Vector2f pt0);
};

ProjectionTextureGenerator::ProjectionTextureGenerator()
  : SampleCollector(new Impl) {}

ProjectionTextureGenerator::~ProjectionTextureGenerator() {}

#define GET_IMPL                                                               \
  auto _impl =                                                                 \
      static_cast<ProjectionTextureGenerator::Impl *>(this->_impl.get())

void ProjectionTextureGenerator::Impl::update(clock::time_point now) {

  SampleCollector::Impl::update(now);
  if (tracker_positions.size() < total_region_positions_count) return;

  GM_INF("ProjectionTextureGenerator",
         "Done collecting all " << total_region_positions_count << " samples");

  saveImage();
}

void ProjectionTextureGenerator::addRegion(size_t order) {
  GET_IMPL;
  _impl->addRegion(order);
}

void ProjectionTextureGenerator::Impl::addRegion(size_t order) {
  region_order.push_back(order);
  region_positions.push_back({});
  region_hull_positions.push_back({});
}

void ProjectionTextureGenerator::addBufferPosition(Eigen::Vector2f p) {
  GET_IMPL;
  if (_impl->region_positions.empty()) addRegion();
  _impl->region_positions.back().push_back(p);
  ++_impl->total_region_positions_count;
}

void ProjectionTextureGenerator::addHullIndex(size_t idx) {
  GET_IMPL;

  if (_impl->region_positions.empty())
    throw gmCore::PreConditionViolation(
        "Cannot set hull index with no points to reference");

  if (idx >= _impl->region_positions.back().size())
    throw gmCore::InvalidArgument(
        GM_STR("Index (" << idx << ") is not a buffer position (0-"
                         << _impl->region_positions.size() << ")"));

  _impl->region_hull_positions.back().push_back(
      _impl->region_positions.back()[idx]);
}

void ProjectionTextureGenerator::addHullPosition(Eigen::Vector2f p) {
  GET_IMPL;
  if (_impl->region_hull_positions.empty()) addRegion();
  _impl->region_hull_positions.back().push_back(p);
}

void ProjectionTextureGenerator::setResolution(gmCore::size2 res) {
  if (res[0] == 0 || res[1] == 0)
    throw gmCore::InvalidArgument("Cannot have zero pixels in resolution");
  GET_IMPL;
  _impl->resolution = res;
}

void ProjectionTextureGenerator::setFile(std::filesystem::path file) {
  if (file.extension() != ".tif" &&
      file.extension() != ".tiff")
    file += ".tif";

  GET_IMPL;
  _impl->file = file;
}

void ProjectionTextureGenerator::saveImage() {
  GET_IMPL;
  _impl->saveImage();
}

void ProjectionTextureGenerator::Impl::saveImage() {

  checkPreconditions();

  GM_DBG1("ProjectionTextureGenerator",
          "Processing with " << total_region_positions_count
                             << "samples to generate image of resolution "
                             << resolution[0] << " x " << resolution[1] << ".");

  size_t tracker_idx = 0;
  std::vector<std::unique_ptr<gmMisc::PolyFit>> polys;

  for (size_t idx = 0; idx < region_order.size(); ++idx){
    auto order = region_order[idx];
    auto &positions = region_positions[idx];

    polys.emplace_back(std::make_unique<gmMisc::PolyFit>(2, 3, order));
    auto &poly = polys.back();

    for (auto pos : positions) {
      poly->addSample(
          {
              pos(0),
              pos(1),
          },
          {
              tracker_positions[tracker_idx](0),
              tracker_positions[tracker_idx](1),
              tracker_positions[tracker_idx](2),
          });
      ++tracker_idx;
    }
  }

  std::vector<gmCore::float3> image_data;
  image_data.reserve(resolution[0] * resolution[1]);

  for (size_t idx_y = 0; idx_y < resolution[1]; ++idx_y)
    for (size_t idx_x = 0; idx_x < resolution[0]; ++idx_x) {

      auto x = idx_x / (double)resolution[0];
      auto y = idx_y / (double)resolution[1];

      size_t closest = getRegionIdx({x, y});
      if (closest == std::numeric_limits<size_t>::max()) {
        auto nan = std::numeric_limits<float>::quiet_NaN();
        image_data.push_back({nan, nan, nan});
      } else {
        auto res = polys[closest]->getValue({x, y});
        image_data.push_back({(float)res[0], (float)res[1], (float)res[2]});
      }
    }

  std::array<float, 3> min = {
      std::numeric_limits<float>::max(),
      std::numeric_limits<float>::max(),
      std::numeric_limits<float>::max(),
  };
  std::array<float, 3> max = {
      std::numeric_limits<float>::min(),
      std::numeric_limits<float>::min(),
      std::numeric_limits<float>::min(),
  };

#define IDX(X,Y) ((X) + resolution[1] * (Y))

  for (size_t idx_y = 0; idx_y < resolution[1]; ++idx_y)
    for (size_t idx_x = 0; idx_x < resolution[0]; ++idx_x)
      for (size_t idx_D = 0; idx_D < 3; ++idx_D) {
        auto res = image_data[IDX(idx_x, idx_y)][idx_D];
        min[idx_D] = std::min(min[idx_D], res);
        max[idx_D] = std::max(max[idx_D], res);
      }

  scale = {max[0] - min[0], max[1] - min[1], max[2] - min[2]};
  offset = {min[0], min[1], min[2]};

  std::array<float, 3> rescale = {
      1.f / scale[0],
      1.f / scale[1],
      1.f / scale[2]
  };

  for (size_t idx_y = 0; idx_y < resolution[1]; ++idx_y)
    for (size_t idx_x = 0; idx_x < resolution[0]; ++idx_x)
      for (size_t idx_D = 0; idx_D < 3; ++idx_D)
        image_data[IDX(idx_x, idx_y)][idx_D] =
            rescale[idx_D] *
            (image_data[IDX(idx_x, idx_y)][idx_D] - offset[idx_D]);

  std::shared_ptr<gmCore::FreeImage> freeimage = gmCore::FreeImage::get();

  FIBITMAP *bitmap =
      FreeImage_AllocateT(FIT_RGBF, resolution[0], resolution[1], 3*32);
  memcpy(FreeImage_GetBits(bitmap),
         image_data.data(),
         resolution[0] * resolution[1] * 3 * 4);

  bool success = FreeImage_Save(FIF_TIFF, bitmap, file.u8string().c_str(), TIFF_LZW);

  FreeImage_Unload(bitmap);

  if (success) {
    GM_INF("ProjectionTextureGenerator",
           "Saved image! Use with <TextureProjectedView warpScale=\""
               << scale.transpose() << "\" warpOffset=\""
               << offset.transpose() << "\"><ImageTexture file=\"" << file
               << "\"/>");
  } else {
    GM_ERR("ProjectionTextureGenerator",
           "Could not save image '" << file << "'!");
  }
}

Eigen::Vector3f ProjectionTextureGenerator::getOffset() {
  GET_IMPL;
  return _impl->offset;
}

Eigen::Vector3f ProjectionTextureGenerator::getScale() {
  GET_IMPL;
  return _impl->scale;
}

void ProjectionTextureGenerator::Impl::checkPreconditions() {
  if (total_region_positions_count == 0 ||
      total_region_positions_count != tracker_positions.size())
    throw gmCore::PreConditionViolation("Wrong number of samples");

  for (size_t idx_R = 0; idx_R < region_order.size(); ++idx_R) {
    size_t order = region_order[idx_R];
    size_t count = region_positions[idx_R].size();
    size_t min_count = (order + 1) * (order + 1);
    if (count < min_count)
      GM_WRN("ProjectionTextureGenerator",
             "Region " << idx_R << " has only " << count << " samples when "
                       << min_count
                       << " is a reasonable minimum for an order of "
                       << order << "!");
  }


  for (size_t idx_R = 0; idx_R < region_order.size(); ++idx_R) {

    if (region_order.size() > 1 && region_hull_positions[idx_R].size() < 3)
      throw gmCore::PreConditionViolation(
          GM_STR("Region " << idx_R << " has invalid number of hull points ("
                           << region_hull_positions[idx_R].size() << ")!"));
    if (region_positions[idx_R].size() < 3)
      throw gmCore::PreConditionViolation(
          GM_STR("Region " << idx_R << " has invalid number of points ("
                           << region_positions[idx_R].size() << ")!"));
  }
}

namespace {
int wind_diff(int q0, int q1, Eigen::Vector2f pt0, Eigen::Vector2f pt1) {

  if (q0 == q1) return 0;

  int q_diff = q1 - q0;
  if (-4 < q_diff && q_diff < 4) return q_diff;
  if (q_diff > 4) return q_diff - 8;
  if (q_diff < -4) return q_diff + 8;

  float a0 = std::atan2(pt0[1], pt0[0]);
  float a1 = std::atan2(pt1[1], pt1[0]);
  if (a0 > a1) return (a0 - a1) > GM_PI ? 4 : -4;
  else return (a1 - a0) < GM_PI ? 4 : -4;
}
}

size_t ProjectionTextureGenerator::Impl::getRegionIdx(Eigen::Vector2f ptq) {

  if (region_positions.size() == 1) return 0;

  size_t region = std::numeric_limits<size_t>::max();

  /// Non-trigonometric winding using octant instead of angle (higher
  /// resolution quadrant) summarizing to 0 or ±8
#define Q(X, Y)                                                                \
  (((X) >= 0.f && (Y) >= 0.f)  ? ((+X) > (+Y) ? 0 : 1)                         \
   : ((X) < 0.f && (Y) >= 0.f) ? ((-X) > (+Y) ? 3 : 2)                         \
   : ((X) < 0.f && (Y) < 0.f)  ? ((-X) > (-Y) ? 4 : 5)                         \
                               : ((+X) > (-Y) ? 7 : 6))

  Eigen::Vector2f pt0, pt1;
  for (size_t idx_R = 0; idx_R < region_order.size(); ++idx_R) {

    pt1 = region_hull_positions[idx_R].back() - ptq;
    int q0 = Q(pt1[0], pt1[1]);

    pt0 = pt1;
    pt1 = region_hull_positions[idx_R].front() - ptq;
    int q1 = Q(pt1[0], pt1[1]);

    int q_sum = wind_diff(q0, q1, pt0, pt1);

    for (size_t idx_H = 1; idx_H < region_hull_positions[idx_R].size(); ++idx_H) {

      q0 = q1;
      pt0 = pt1;

      pt1 = region_hull_positions[idx_R][idx_H] - ptq;
      q1 = Q(pt1[0], pt1[1]);

      q_sum += wind_diff(q0, q1, pt0, pt1);
    }

    if (std::fabs(q_sum) == 8) region = idx_R;
  }

  return region;
}

END_NAMESPACE_GMTRACK;

#endif
