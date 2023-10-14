
#include <gmTrack/ArucoGridBoard.hh>

#ifdef gramods_ENABLE_OpenCV_objdetect

#include <gmCore/Console.hh>
#include <gmCore/RunOnce.hh>

#include <optional>

BEGIN_NAMESPACE_GMTRACK;

GM_OFI_DEFINE(ArucoGridBoard);
GM_OFI_PARAM2(ArucoGridBoard, columns, size_t, setColumns);
GM_OFI_PARAM2(ArucoGridBoard, rows, size_t, setRows);
GM_OFI_PARAM2(ArucoGridBoard, markerSize, float, setMarkerSize);
GM_OFI_PARAM2(ArucoGridBoard, markerSeparation, float, setMarkerSeparation);
GM_OFI_PARAM2(ArucoGridBoard, firstId, size_t, setFirstId);
GM_OFI_PARAM2(ArucoGridBoard, dictionary, std::string, setDictionary);
GM_OFI_PARAM2(ArucoGridBoard, position, Eigen::Vector3f, setPosition);
GM_OFI_PARAM2(ArucoGridBoard, orientation, Eigen::Quaternionf, setOrientation);


struct ArucoGridBoard::Impl {

  cv::Ptr<cv::aruco::Board> getBoard();

  cv::Ptr<cv::aruco::Board> cache_board;
  bool cache_up_to_date = false;

  static std::optional<cv::aruco::Dictionary> getDictionary(std::string name);

  size_t columns = 1;
  size_t rows = 1;
  float size = -1;
  float sep = -1;
  size_t id_0 = 0;
  cv::aruco::Dictionary dict = cv::aruco::getPredefinedDictionary(0);

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
    _impl->dict = *newdict;
  else
    GM_WRN("ArucoGridBoard", "Could not set dictionary to unrecognized value '" << dict << "'.");
}

std::optional<cv::aruco::Dictionary>
ArucoGridBoard::Impl::getDictionary(std::string name) {

#  define DICT(CODE)                                                           \
    if (name == #CODE)                                                  \
    return cv::aruco::getPredefinedDictionary(cv::aruco::DICT_##CODE)

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

  return std::nullopt;

#undef DICT
}

void ArucoGridBoard::setPosition(Eigen::Vector3f p) {
  _impl->cache_up_to_date = false;
  _impl->position = p;
}

void ArucoGridBoard::setOrientation(Eigen::Quaternionf q) {
  _impl->cache_up_to_date = false;
  _impl->orientation = q;
}

cv::Ptr<cv::aruco::Board> ArucoGridBoard::getBoard() {
  return _impl->getBoard();
}

cv::Ptr<cv::aruco::Board> ArucoGridBoard::Impl::getBoard() {

  if (cache_up_to_date)
    return cache_board;

  cache_up_to_date = true;
  cache_board = nullptr;

  if (size <= 0) {
    GM_WRN("ArucoGridBoard", "Marker size not specified - using 5 cm.");
    size = 0.05f;
  }

  if (columns * rows > 1 && sep <= 0) {
    GM_WRN("ArucoGridBoard", "Separation not specified - using 0.5 x marker size.");
    sep = 0.5f * size;
  }

  std::vector<int> all_ids; all_ids.reserve(columns * rows);
  for (int idx = 0; idx < columns * rows; ++idx) all_ids.push_back(idx + id_0);

  auto aboard =
      cv::aruco::GridBoard({int(columns), int(rows)}, size, sep, dict, all_ids);

  std::vector<std::vector<cv::Point3f>> objPoints;
  std::vector<int> ids;

  auto &aboard_objPoints = aboard.getObjPoints();
  auto &p0 = aboard_objPoints[0][0];
  auto &p1 = aboard_objPoints[columns - 1][1];
  auto &p2 = aboard_objPoints[columns * rows - 1][2];
  auto &p3 = aboard_objPoints[columns *(rows - 1)][3];

  cv::Point3f cp = 0.25 * (p0 + p1 + p2 + p3);

  for (auto opt : aboard_objPoints) {
    std::vector<cv::Point3f> pts;
    for (auto pt : opt) {
      pt -= cp;
      // Move and rotate in OpenGL axes convention
      Eigen::Vector3f ept(pt.x, -pt.y, -pt.z);
      ept = orientation * ept + position;
      pts.push_back(cv::Point3f(ept.x(), -ept.y(), -ept.z()));
    }
    objPoints.push_back(pts);
  }
  ids.insert(ids.end(), aboard.getIds().begin(), aboard.getIds().end());

  cache_board = new cv::aruco::Board(objPoints, dict, ids);
  return cache_board;
}

END_NAMESPACE_GMTRACK;

#endif
