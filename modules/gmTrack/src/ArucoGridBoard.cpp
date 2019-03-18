
#include <gmTrack/ArucoGridBoard.hh>

#ifdef gramods_ENABLE_aruco

#include <gmCore/RunOnce.hh>

BEGIN_NAMESPACE_GMTRACK;

GM_OFI_DEFINE(ArucoGridBoard);
GM_OFI_PARAM(ArucoGridBoard, columns, size_t, ArucoGridBoard::setColumns);
GM_OFI_PARAM(ArucoGridBoard, rows, size_t, ArucoGridBoard::setRows);
GM_OFI_PARAM(ArucoGridBoard, markerSize, float, ArucoGridBoard::setMarkerSize);
GM_OFI_PARAM(ArucoGridBoard, markerSeparation, float, ArucoGridBoard::setMarkerSeparation);
GM_OFI_PARAM(ArucoGridBoard, firstId, size_t, ArucoGridBoard::setFirstId);
GM_OFI_PARAM(ArucoGridBoard, dictionary, std::string, ArucoGridBoard::setDictionary);
GM_OFI_PARAM(ArucoGridBoard, position, gmTypes::float3, ArucoGridBoard::setPosition);
GM_OFI_PARAM(ArucoGridBoard, quaternion, gmTypes::float4, ArucoGridBoard::setQuaternion);
GM_OFI_PARAM(ArucoGridBoard, axisAngle, gmTypes::float4, ArucoGridBoard::setAxisAngle);
GM_OFI_PARAM(ArucoGridBoard, eulerAngles, gmTypes::float3, ArucoGridBoard::setEulerAngles);


struct ArucoGridBoard::Impl {

  cv::Ptr<cv::aruco::Board> getBoard();

  cv::Ptr<cv::aruco::Board> cache_board;
  bool cache_up_to_date = false;

  static cv::Ptr<cv::aruco::Dictionary> getDictionary(std::string name);

  size_t columns = 1;
  size_t rows = 1;
  float size = 0.05;
  float sep = 0.05;
  size_t id_0 = 0;
  cv::Ptr<cv::aruco::Dictionary> dict = getDictionary("ARUCO_ORIGINAL");

  Eigen::Vector3f position = Eigen::Vector3f::Zero();
  Eigen::Quaternionf orientation = Eigen::Quaternionf::Identity();
};

ArucoGridBoard::ArucoGridBoard()
  : _impl(std::make_unique<Impl>()) {}


void ArucoGridBoard::setColumns(size_t N) {
  _impl->cache_up_to_date = false;
  _impl->columns = N;
}

void ArucoGridBoard::setRows(size_t N) {
  _impl->cache_up_to_date = false;
  _impl->rows = N;
}

void ArucoGridBoard::setMarkerSize(float s) {
  _impl->cache_up_to_date = false;
  _impl->size = s;
}

void ArucoGridBoard::setMarkerSeparation(float s) {
  _impl->cache_up_to_date = false;
  _impl->sep = s;
}

void ArucoGridBoard::setFirstId(size_t id) {
  _impl->cache_up_to_date = false;
  _impl->id_0 = id;
}

void ArucoGridBoard::setDictionary(std::string dict) {
  _impl->cache_up_to_date = false;
  auto newdict = Impl::getDictionary(dict);
  if (newdict)
    _impl->dict = newdict;
  else
    GM_WRN("ArucoGridBoard", "Could not set dictionary to unrecognized value '" << dict << "'.");
}

cv::Ptr<cv::aruco::Dictionary> ArucoGridBoard::Impl::getDictionary(std::string name) {

#define DICT(CODE) if (name == #CODE) return cv::aruco::Dictionary::get(cv::aruco::DICT_##CODE)

  DICT(4X4_50);
  DICT(4X4_100);
  DICT(4X4_250);
  DICT(4X4_1000);
  DICT(5X5_50);
  DICT(5X5_100);
  DICT(5X5_250);
  DICT(5X5_1000);
  DICT(6X6_50);
  DICT(6X6_100);
  DICT(6X6_250);
  DICT(6X6_1000);
  DICT(7X7_50);
  DICT(7X7_100);
  DICT(7X7_250);
  DICT(7X7_1000);
  DICT(ARUCO_ORIGINAL);
  DICT(APRILTAG_16h5);
  DICT(APRILTAG_25h9);
  DICT(APRILTAG_36h10);
  DICT(APRILTAG_36h11);

  return nullptr;

#undef DICT

}

void ArucoGridBoard::setPosition(gmTypes::float3 p) {
  _impl->cache_up_to_date = false;
  _impl->position = Eigen::Vector3f(p[0], p[1], p[2]);
}

void ArucoGridBoard::setQuaternion(gmTypes::float4 q) {
  _impl->cache_up_to_date = false;
  _impl->orientation = Eigen::Quaternionf(q[0], q[1], q[2], q[3]);
}

void ArucoGridBoard::setAxisAngle(gmTypes::float4 aa) {
  _impl->cache_up_to_date = false;
  Eigen::Quaternionf Q(Eigen::Quaternionf::AngleAxisType
                       (aa[3], Eigen::Vector3f(aa[0], aa[1], aa[2]).normalized()));
  _impl->orientation = Q;
}

void ArucoGridBoard::setEulerAngles(gmTypes::float3 ea) {
  _impl->cache_up_to_date = false;
  Eigen::Quaternionf Q(Eigen::AngleAxisf(ea[0], Eigen::Vector3f::UnitX()) *
                       Eigen::AngleAxisf(ea[1], Eigen::Vector3f::UnitY()) *
                       Eigen::AngleAxisf(ea[2], Eigen::Vector3f::UnitZ()));
  _impl->orientation = Q;
}

cv::Ptr<cv::aruco::Board> ArucoGridBoard::getBoard() {
  return _impl->getBoard();
}

cv::Ptr<cv::aruco::Board> ArucoGridBoard::Impl::getBoard() {

  if (cache_up_to_date)
    return cache_board;

  cache_up_to_date = true;
  cache_board = nullptr;

  auto aboard = cv::aruco::GridBoard::create(columns, rows, size, sep, dict, id_0);
  /*
  std::vector<std::vector<cv::Point3f>> objPoints;
  std::vector<int> ids;

  objPoints.insert(objPoints.end(), aboard->objPoints.begin(), aboard->objPoints.end());
  ids.insert(ids.end(), aboard->ids.begin(), aboard->ids.end());
*/
  cache_board = aboard;//cv::aruco::Board::create(objPoints, dict, ids);
  return cache_board;
}

END_NAMESPACE_GMTRACK;

#endif
