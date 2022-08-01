
#include <gmGraphics/EacCoordinatesMapper.hh>

#include <gmCore/MathConstants.hh>

BEGIN_NAMESPACE_GMGRAPHICS;

GM_OFI_DEFINE(EacCoordinatesMapper);
GM_OFI_PARAM2(EacCoordinatesMapper, layout, std::string, setLayout);

struct EacCoordinatesMapper::Impl {

  void setMapperUniforms(GLuint program_id);
  void setLayout(std::string layout);

  std::vector<float> data_M_to2D;
  std::vector<GLint> pos_col = {-1, -1, -1, -1, -1, -1};
  std::vector<GLint> pos_row = {-1, -1, -1, -1, -1, -1};

  std::vector<float> data_M_to3D;
  size_t rows;
  size_t cols;
};

EacCoordinatesMapper::EacCoordinatesMapper()
  : _impl(std::make_unique<Impl>()) {
  setLayout("LFR/D-B-U-");
}
EacCoordinatesMapper::~EacCoordinatesMapper() {}

std::string EacCoordinatesMapper::getMapperCode() {
  static const std::string code = R"lang=glsl(

#define PI1 3.14159265358979323846264338327950288419716939937511
#define PI2 1.57079632679489661923132169163975144209858469968755

uniform int rows;
uniform int cols;

uniform mat3 M_to2D[6];
uniform int pos_row[6];
uniform int pos_col[6];

bool mapTo2D(vec3 pos3, out vec2 pos2) {

  float abs_x = abs(pos3.x);
  float abs_y = abs(pos3.y);
  float abs_z = abs(pos3.z);

  int section;
       if (pos3.x < -abs_y && pos3.x < -abs_z) section = 0;
  else if (pos3.z < -abs_x && pos3.z < -abs_y) section = 1;
  else if (pos3.x >  abs_y && pos3.x >  abs_z) section = 2;
  else if (pos3.y >  abs_x && pos3.y >  abs_z) section = 3;
  else if (pos3.z >  abs_x && pos3.z >  abs_y) section = 4;
  else section = 5;

  pos3 = M_to2D[section] * pos3;

  // (2/pi)*arctan(2*x) [-0.5, 0.5] -> [-0.5, 0.5]
  // Find angles φ_x∊[-π,π] and φ_y∊[-π,π]
  float phi_x = atan(pos3.x, -pos3.z);
  float phi_y = atan(pos3.y, -pos3.z);

  // Go from [-π/4,π/4] to [-1,1]
  pos2 = vec2((1.0 / PI2) * phi_x + 0.5, (1.0 / PI2) * phi_y + 0.5);

  pos2.x = (1.0 / float(cols)) * pos2.x + pos_col[section] / float(cols);
  int row = rows - pos_row[section] - 1;
  pos2.y = (1.0 / float(rows)) * pos2.y + row              / float(rows);

  pos2 = 2.0 * pos2 - 1.0;

  return true;
}

uniform mat3 M_to3D[6];

bool mapTo3D(vec2 pos2, out vec3 pos3) {

  // Go from [-1,1] to [0,1] for simplicity
  pos2 = 0.5 * (pos2 + 1.0);

  int section = 0;
  for (int idx = 0; idx < cols; ++idx) {
    if (pos2.x > (idx + 1.0) / cols) continue;
    pos2.x = cols * (pos2.x - idx / float(cols));
    section = idx;
    break;
  }

  for (int idx = 0; idx < rows; ++idx) {
    if (pos2.y > (idx + 1.0) / rows) continue;
    int row = rows - idx - 1;
    pos2.y = rows * (pos2.y - idx / float(rows));
    section = section + row * cols;
    break;
  }

  // (2/pi)*arctan(2*x) [-0.5, 0.5] -> [-0.5, 0.5]
  // [0,1] -> [-1,1] (via tan)
  float ax = tan(PI2 * (pos2.x - 0.5));
  float ay = tan(PI2 * (pos2.y - 0.5));

  // (0,0) = -z, ax = +x, ay = +y
  pos3 = vec3(ax, ay, -1.0);
  pos3 = M_to3D[section] * normalize(pos3);

  return true;
}

)lang=glsl";
  return code;
}

void EacCoordinatesMapper::setMapperUniforms(GLuint program_id) {
  _impl->setMapperUniforms(program_id);
}

void EacCoordinatesMapper::Impl::setMapperUniforms(GLuint program_id) {
  glUniform1i(glGetUniformLocation(program_id, "rows"), GLint(rows));
  glUniform1i(glGetUniformLocation(program_id, "cols"), GLint(cols));

  glUniformMatrix3fv(glGetUniformLocation(program_id, "M_to2D"),
                     data_M_to2D.size() / 9,
                     false,
                     data_M_to2D.data());
  glUniform1iv(glGetUniformLocation(program_id, "pos_row"),
               pos_row.size(),
               pos_row.data());
  glUniform1iv(glGetUniformLocation(program_id, "pos_col"),
               pos_col.size(),
               pos_col.data());

  glUniformMatrix3fv(glGetUniformLocation(program_id, "M_to3D"),
                     data_M_to3D.size() / 9,
                     false,
                     data_M_to3D.data());
}

void EacCoordinatesMapper::setLayout(std::string layout) {
  _impl->setLayout(layout);
}

void EacCoordinatesMapper::Impl::setLayout(std::string layout) {

  static const std::unordered_map<char, size_t> index = {
      {'L', 0}, {'F', 1}, {'R', 2}, {'U', 3}, {'B', 4}, {'D', 5}};

  rows = 0;
  cols = 0;

  std::vector<Eigen::Matrix3f> M_to3D;
  M_to3D.reserve(6);

  size_t curr_col = 0, curr_row = 0;
  for (const auto C : layout) {
    switch (C) {
    case 'F':
      pos_col[index.at(C)] = curr_col;
      pos_row[index.at(C)] = curr_row;
      ++curr_col;
      M_to3D.push_back(Eigen::Matrix3f::Identity());
      break;
    case 'L':
      pos_col[index.at(C)] = curr_col;
      pos_row[index.at(C)] = curr_row;
      ++curr_col;
      M_to3D.push_back(
          Eigen::AngleAxisf(GM_PI_2, Eigen::Vector3f::UnitY()).matrix());
      break;
    case 'R':
      pos_col[index.at(C)] = curr_col;
      pos_row[index.at(C)] = curr_row;
      ++curr_col;
      M_to3D.push_back(
          Eigen::AngleAxisf(-GM_PI_2, Eigen::Vector3f::UnitY()).matrix());
      break;
    case 'B':
      pos_col[index.at(C)] = curr_col;
      pos_row[index.at(C)] = curr_row;
      ++curr_col;
      M_to3D.push_back(
          Eigen::AngleAxisf(GM_PI, Eigen::Vector3f::UnitY()).matrix());
      break;
    case 'U':
      pos_col[index.at(C)] = curr_col;
      pos_row[index.at(C)] = curr_row;
      ++curr_col;
      M_to3D.push_back((Eigen::AngleAxisf(GM_PI, Eigen::Vector3f::UnitY()) *
                        Eigen::AngleAxisf(GM_PI_2, Eigen::Vector3f::UnitX()))
                           .matrix());
      break;
    case 'D':
      pos_col[index.at(C)] = curr_col;
      pos_row[index.at(C)] = curr_row;
      ++curr_col;
      M_to3D.push_back((Eigen::AngleAxisf(GM_PI, Eigen::Vector3f::UnitY()) *
                        Eigen::AngleAxisf(-GM_PI_2, Eigen::Vector3f::UnitX()))
                           .matrix());
      break;
    case '-':
      if (M_to3D.empty())
        throw gmCore::InvalidArgument(
            "Cannot rotate last square - does not exist.");
      M_to3D.back() =
          M_to3D.back() * Eigen::AngleAxisf(GM_PI_2, Eigen::Vector3f::UnitZ());
      break;
    case '+':
      if (M_to3D.empty())
        throw gmCore::InvalidArgument(
            "Cannot rotate last square - does not exist.");
      M_to3D.back() =
          M_to3D.back() * Eigen::AngleAxisf(-GM_PI_2, Eigen::Vector3f::UnitZ());
      break;
    case '/':
      if (cols == 0)
        cols = curr_col;
      else if (curr_col != cols)
        throw gmCore::InvalidArgument(GM_STR(
            "Inconsistent column count (" << curr_col << " on row " << curr_row
                                          << " != " << cols << ")."));
      curr_col = 0;
      ++curr_row;
      break;
    default:
      throw gmCore::InvalidArgument(
          GM_STR("Char '" << C << "' is not valid in a EAC layout."));
    }
  }

  if (cols == 0)
    cols = curr_col;
  else if (curr_col != cols)
    throw gmCore::InvalidArgument(GM_STR("Inconsistent column count ("
                                         << curr_col << " on row " << curr_row
                                         << " != " << cols << ")."));
  rows = curr_row + 1;

  if (cols * rows != 6)
    throw gmCore::InvalidArgument(
        GM_STR("Incorrect cell count; must be 6 (" << (cols * rows) << ")."));

  std::vector<Eigen::Matrix3f> M_to2D(6);
  for (const auto idx : index) {
    if (pos_col[idx.second] < 0 || pos_row[idx.second] < 0)
      throw gmCore::InvalidArgument(
          GM_STR("Missing direction in layout (" << idx.first << ")."));
    auto col = pos_col[idx.second];
    auto row = pos_row[idx.second];
    M_to2D[idx.second] = M_to3D[col + row * cols].inverse();
  }

  data_M_to3D.clear();
  data_M_to3D.reserve(9 * M_to3D.size());
  for (const auto &M : M_to3D)
    data_M_to3D.insert(data_M_to3D.end(), M.data(), M.data() + M.size());

  data_M_to2D.clear();
  data_M_to2D.reserve(9 * M_to2D.size());
  for (const auto &M : M_to2D)
    data_M_to2D.insert(data_M_to2D.end(), M.data(), M.data() + M.size());
}

END_NAMESPACE_GMGRAPHICS;
