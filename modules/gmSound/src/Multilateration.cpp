
#include <gmSound/Multilateration.hh>

#include <gmCore/io_eigen.hh>
#include <gmCore/Console.hh>
#include <gmCore/RunOnce.hh>
#include <gmCore/PreConditionViolation.hh>
#include <gmCore/TimeTools.hh>

#include <gmMisc/NelderMead.hh>

namespace gramods { namespace gmSound {

GM_OFI_DEFINE(Multilateration);
GM_OFI_PARAM2(Multilateration, point, Eigen::Vector3f, addPoint);
GM_OFI_PARAM2(Multilateration, speedOfSound, float, setSpeedOfSound);
GM_OFI_POINTER2(Multilateration, soundDetector, gmSound::SoundDetector, setSoundDetector);

struct Multilateration::Impl {
  void update(clock::time_point t);
  void initialize();

  void estimatePose(std::vector<float> &offsets,
                    clock::time_point time);

  std::optional<PoseSample> pose;

  std::vector<Eigen::Vector3f> points;
  float max_mic_dist = 0;

  float speed_of_sound = 343;
  std::shared_ptr<SoundDetector> sound_detector;
};

Multilateration::Multilateration()
  : gmCore::Updateable(10), _impl(std::make_unique<Impl>()) {}
Multilateration::~Multilateration() {}

void Multilateration::addPoint(Eigen::Vector3f pt) {
  _impl->points.push_back(pt);
}

void Multilateration::setSpeedOfSound(float v) { _impl->speed_of_sound = v; }

void Multilateration::setSoundDetector(std::shared_ptr<SoundDetector> sd) {
  _impl->sound_detector = sd;
}

bool Multilateration::getPose(PoseSample &p) {
  if (!_impl->pose) return false;

  p = *_impl->pose;
  return true;
}

void Multilateration::update(clock::time_point t, size_t frame) {
  _impl->update(t);
}

void Multilateration::initialize() {
  gmTrack::SinglePoseTracker::initialize();
  _impl->initialize();
}

void Multilateration::Impl::initialize() {
  max_mic_dist = 0.f;
  for (const auto &pt1 : points)
    for (const auto &pt2 : points)
      max_mic_dist = std::max(max_mic_dist, (pt1 - pt2).norm());
}

void Multilateration::Impl::update(clock::time_point time) {
  if (!sound_detector) {
    GM_RUNONCE(GM_ERR(
        "Multilateration",
        "Cannot read data without a sound detector instance to read data from!"));
    return;
  }

  const auto &offsets = sound_detector->detectSound();
  if (offsets.empty()) return;

  std::vector<float> first_offsets;
  first_offsets.reserve(offsets.size());
  for (const auto &co : offsets) {
    if (co.empty()) continue;
    first_offsets.push_back(co.front());
  }

  if (first_offsets.size() != points.size()) return;

  estimatePose(first_offsets, time);
}

void Multilateration::Impl::estimatePose(std::vector<float> &offsets,
                                         clock::time_point time) {

  // Time offset (from now) to the first microphone
  float t0 = offsets[0];
  size_t t0_idx = 0;
  for (size_t idx = 1; idx < offsets.size(); ++idx) {
    const auto &value = offsets[idx];
    if (value <= t0) continue;
    t0 = value;
    t0_idx = idx;
  }

  // Additional distance to source relative first microphone
  std::vector<float> distances;
  distances.reserve(offsets.size());
  for (const auto &t : offsets) distances.push_back((t0 - t) * speed_of_sound);

  // Initial simplex based on the microphone positions
  std::vector<Eigen::Vector4f> x0;
  for (const auto &pt : points)
    x0.push_back({pt[0], pt[1], pt[2], 0.2f * max_mic_dist});
  {
    auto const &pt = points.back();
    x0.push_back({pt[0], pt[1], pt[2], 0.8f * max_mic_dist});
  }

  size_t iterations = 0;

  auto res4 = gmMisc::NelderMead::solve<float>(
      x0,
      [&, this](const Eigen::Vector4f &x) {
        Eigen::Vector3f p = x.block<3, 1>(0, 0);
        float d0 = x[3];

        float err2 = 0.f;
        for (size_t idx = 0; idx < points.size(); ++idx) {
          // Mic distance to source should be relative distance plus
          // distance to first microphone
          float err = (points[idx] - p).norm() - (distances[idx] + d0);
          err2 += err * err;
        }
        return err2;
      },
      iterations);

  const auto pt = res4.block<3, 1>(0, 0); // Microphone position
  const auto d0 = res4[3]; // Distance from source to first microphone

  // Incorrect distance to first mic indicates error
  const float err = std::fabs((pt - points[t0_idx]).norm() - d0);
  if (err > 10 * max_mic_dist * std::numeric_limits<float>::epsilon()) return;

  // Sound originated some time ago
  const auto dt = t0 + d0 / speed_of_sound;
  GM_DBG2("Multilateration",
         "Time offset: t0=" << t0 << ", d0=" << d0 << " -> " << dt << " seconds ");

  pose = {pt,
          Eigen::Quaternionf::Identity(),
          time - gmCore::TimeTools::secondsToDuration(dt)};
}

void Multilateration::traverse(Visitor *visitor) {
  if (_impl->sound_detector) _impl->sound_detector->accept(visitor);
}

}}
