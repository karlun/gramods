
#include <gmTrack/ArucoBoardComplex.hh>

#ifdef gramods_ENABLE_aruco

#include <gmCore/RunOnce.hh>

BEGIN_NAMESPACE_GMTRACK;

GM_OFI_DEFINE(ArucoBoardComplex);
GM_OFI_POINTER(ArucoBoardComplex, arucoBoard, gmTrack::ArucoBoard, ArucoBoardComplex::addArucoBoard);
GM_OFI_PARAM(ArucoBoardComplex, position, gmTypes::float3, ArucoBoardComplex::addPosition);
GM_OFI_PARAM(ArucoBoardComplex, quaternion, gmTypes::float4, ArucoBoardComplex::addQuaternion);
GM_OFI_PARAM(ArucoBoardComplex, axisAngle, gmTypes::float4, ArucoBoardComplex::addAxisAngle);


struct ArucoBoardComplex::Impl {

  cv::Ptr<cv::aruco::Board> getBoard();

  cv::Ptr<cv::aruco::Board> cache_board;
  bool cache_up_to_date = false;

  std::vector<std::shared_ptr<gmTrack::ArucoBoard>> boards;

  std::vector<Eigen::Vector3f> positions;

  std::vector<Eigen::Quaternionf> orientations;
};

ArucoBoardComplex::ArucoBoardComplex()
  : _impl(std::make_unique<Impl>()) {}

void ArucoBoardComplex::addArucoBoard(std::shared_ptr<ArucoBoard> b) {
  _impl->cache_up_to_date = false;
  _impl->boards.push_back(b);
}

void ArucoBoardComplex::addPosition(gmTypes::float3 p) {
  _impl->cache_up_to_date = false;
  _impl->positions.push_back(Eigen::Vector3f(p[0], p[1], p[2]));
}

void ArucoBoardComplex::addQuaternion(gmTypes::float4 q) {
  _impl->cache_up_to_date = false;
  _impl->orientations.push_back(Eigen::Quaternionf(q[0], q[1], q[2], q[3]));
}

void ArucoBoardComplex::addAxisAngle(gmTypes::float4 q) {
  _impl->cache_up_to_date = false;
  Eigen::Quaternionf Q(Eigen::Quaternionf::AngleAxisType
                       (q[3], Eigen::Vector3f(q[0], q[1], q[2]).normalized()));
  _impl->orientations.push_back(Q);
}

cv::Ptr<cv::aruco::Board> ArucoBoardComplex::getBoard() {
  return _impl->getBoard();
}

cv::Ptr<cv::aruco::Board> ArucoBoardComplex::Impl::getBoard() {

  if (cache_up_to_date)
    return cache_board;

  cache_up_to_date = true;
  cache_board = nullptr;

  if (boards.empty()) {
    GM_RUNONCE(GM_ERR("ArucoBoardComplex", "Incorrect data - there must be at least one board, position and orientation."));
    return nullptr;
  }

  if ((!positions.empty() && boards.size() != positions.size()) ||
      (!orientations.empty() && boards.size() != orientations.size())) {
    GM_RUNONCE(GM_ERR("ArucoBoardComplex", "Incorrect data - number of boards, positions and orientations must be identical."));
    return nullptr;
  }

  std::vector<cv::Ptr<cv::aruco::Board>> aboards;
  for (auto b : boards)
    aboards.push_back(b->getBoard());
  
  auto dictionary = aboards[0]->dictionary;
  for (auto b : aboards)
    if (b->dictionary != dictionary) {
      GM_RUNONCE(GM_ERR("ArucoBoardComplex", "Incorrect data - cannot create complex of boards that use different dictionaries."));
      return nullptr;
    }

  std::vector<std::vector<cv::Point3f>> objPoints;
  std::vector<int> ids;

  for (int idx = 0; idx < aboards.size(); ++idx) {

    auto aboard = aboards[idx];
    auto position = positions.empty() ?
      Eigen::Vector3f::Zero() :
      positions[idx];
    auto orientation =
      orientations.empty() ?
      Eigen::Quaternionf::Identity() :
      orientations[idx];

    for (auto opt : aboard->objPoints) {
      std::vector<cv::Point3f> pts;
      for (auto pt : opt) {
        Eigen::Vector3f ept(pt.x, pt.y, pt.z);
        ept = orientation * ept + position;
        pts.push_back(cv::Point3f(ept.x(), ept.y(), ept.z()));
      }
      objPoints.push_back(pts);
    }
    ids.insert(ids.end(), aboard->ids.begin(), aboard->ids.end());
  }

  cache_board = cv::aruco::Board::create(objPoints, dictionary, ids);
  return cache_board;
}

END_NAMESPACE_GMTRACK;

#endif
