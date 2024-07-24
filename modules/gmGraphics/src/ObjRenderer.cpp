
#include <gmGraphics/ObjRenderer.hh>

#ifdef gramods_ENABLE_tinyobjloader

#include <gmGraphics/AABB.hh>
#include <gmGraphics/GLUtils.hh>

#include <gmCore/Console.hh>
#include <gmCore/RunOnce.hh>
#include <gmCore/MathConstants.hh>
#include <gmCore/FileResolver.hh>

#include <GL/glew.h>
#include <GL/gl.h>

#include <limits>
#include <optional>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

BEGIN_NAMESPACE_GMGRAPHICS;

GM_OFI_DEFINE_SUB(ObjRenderer, Renderer);
GM_OFI_PARAM2(ObjRenderer, file, std::filesystem::path, setFile);
GM_OFI_PARAM2(ObjRenderer, recenter, bool, setRecenter);

struct ObjRenderer::Impl {

  struct MyHash {
    std::size_t operator()(const std::array<int, 3> &v) const noexcept {
      return v[0] ^ (v[1] << 1) ^ (v[2] >> 1);
    }
  };

  typedef std::unordered_map<std::array<int, 3>, size_t, MyHash> index_map_t;

  ~Impl();

  void load_data();
  void setup();
  bool read_obj(std::vector<GLfloat> &vertices,
                std::vector<GLfloat> &normals,
                std::vector<GLfloat> &tcoords,
                std::vector<GLint> &mtls);
  void addTriangle(std::vector<GLfloat> &vertices,
                   std::vector<GLfloat> &normals,
                   std::vector<GLfloat> &tcoords,
                   std::vector<GLint> &mtls,
                   index_map_t &index_map,
                   const tinyobj::attrib_t &attrib,
                   const tinyobj::mesh_t &mesh);
  Material makeMaterial(const tinyobj::material_t &mat);
  void render(const Camera &camera, const Eigen::Affine3f &Mm);
  void getNearFar(const Camera &camera,
                  const Eigen::Affine3f &Mm,
                  float &near,
                  float &far);
  void teardown();

  static const std::string vertex_shader_code;
  static const std::string fragment_shader_code;
  static const std::string mapper_pattern;

  GLuint vertex_shader_id = 0;
  GLuint fragment_shader_id = 0;
  GLuint program_id = 0;
  GLuint vao_id = 0;
  GLuint vbo_id[4] = { 0, 0, 0, 0 };
  std::vector<Material> materials;
  std::vector<std::vector<GLuint>> triangle_indices;

  struct uniforms {
    GLint Mp = 0;
    GLint Mv = 0;
    GLint Mn = 0;
    GLint Mm = 0;

    GLint texture_active = 0;

    GLint texture_ambient = 0;
    GLint texture_diffuse = 0;
    GLint texture_specular = 0;
    GLint texture_emissive = 0;

    GLint color_ambient = 0;
    GLint color_diffuse = 0;
    GLint color_specular = 0;
    GLint color_emissive = 0;
    GLint shininess = 0;
  };

  std::unordered_map<GLint, uniforms> loc;

  std::vector<GLfloat> vertices;
  std::vector<GLfloat> normals;
  std::vector<GLfloat> tcoords;
  std::vector<GLint> mtls;

  std::vector<float> getIntersections(const IntersectionLine &line);
  std::optional<AABB> bounds;

  std::filesystem::path file;
  std::filesystem::path tex_root_path;
  bool recenter = false;

  bool is_setup = false;
  bool is_functional = false;
};

ObjRenderer::ObjRenderer()
  : _impl(std::make_unique<Impl>()) {}
ObjRenderer::~ObjRenderer() {}

void ObjRenderer::initialize() {
  Renderer::initialize();
  _impl->load_data();
}

void ObjRenderer::Impl::load_data() {

  vertices.clear();
  normals.clear();
  tcoords.clear();
  mtls.clear();

  if (!read_obj(vertices, normals, tcoords, mtls)) return;

  if (vertices.size() < 3) {
    GM_ERR("ObjRenderer", "No vertices read");
    return;
  }

  bounds = AABB({vertices[0], vertices[1], vertices[2]});
  for (size_t idx = 3; idx + 2 < vertices.size(); idx += 3)
    *bounds +=
        Eigen::Vector3f(vertices[idx], vertices[idx + 1], vertices[idx + 2]);

  if (recenter) {
    Eigen::Vector3f offset = bounds->getCenter();
    for (size_t idx = 0; idx + 2 < vertices.size(); idx += 3) {
      vertices[idx + 0] -= offset.x();
      vertices[idx + 1] -= offset.y();
      vertices[idx + 2] -= offset.z();
    }
    bounds->addOffset(-offset);
  }

  GM_DBG1("ObjRenderer",
          "Initialized with "
              << vertices.size() << " vertices, " << normals.size()
              << " normals and " << tcoords.size() << " texture coordinates\n"
              << "Bounds: " << bounds->min().transpose() << " - "
              << bounds->max().transpose());
}

void ObjRenderer::render(const Camera &camera, const Eigen::Affine3f &Mm) {
  if (!eyes.empty() && eyes.count(camera.getEye()) == 0) return;
  _impl->render(camera, Mm);
}

void ObjRenderer::getNearFar(const Camera &camera,
                             const Eigen::Affine3f &Mm,
                             float &near,
                             float &far) {
  if (!eyes.empty() && eyes.count(camera.getEye()) == 0) return;
  _impl->getNearFar(camera, Mm, near, far);
}

void ObjRenderer::Impl::getNearFar(const Camera &camera,
                                   const Eigen::Affine3f &Mm,
                                   float &near,
                                   float &far) {

  if (!bounds) return;

  auto Mv = camera.getViewMatrix();

  near = std::numeric_limits<float>::max();
  far = std::numeric_limits<float>::min();

  auto T = Mv * Mm;
  for (auto const &pt : bounds->getCorners()) {
    const float z = -(T * pt.homogeneous()).z();
    near = std::min(near, z);
    far = std::max(far, z);
  }
}

const std::string ObjRenderer::Impl::vertex_shader_code = R"lang=glsl(
#version 330 core

uniform mat4 Mp;
uniform mat4 Mv;
uniform mat3 Mn;
uniform mat4 Mm;

const vec3 light1 = vec3(0.5773, 0.5773, -0.5773);
const vec3 light2 = vec3(0, 0.7071, 0.7071);

layout (location = 0) in vec4 vert_position;
layout (location = 1) in vec3 vert_normal;
layout (location = 2) in vec2 vert_tcoord;
layout (location = 3) in int vert_mtl;

out vec3 frag_pos;
out vec3 frag_normal;
out vec2 frag_tcoord;
flat out int frag_mtl;
out vec3 local_light1;
out vec3 local_light2;

void main() {
  gl_Position = Mp * Mv * Mm * vert_position;

  local_light1 = normalize(mat3(Mv) * light1);
  local_light2 = normalize(mat3(Mv) * light2);

  frag_pos = (Mv * Mm * vert_position).xyz;
  frag_normal = normalize(mat3(Mv) * Mn * vert_normal);
  frag_tcoord = vert_tcoord;
  frag_mtl = vert_mtl;
}
)lang=glsl";

const std::string ObjRenderer::Impl::fragment_shader_code = R"lang=glsl(
#version 330 core

uniform int texture_active;

uniform sampler2D texture_ambient;
uniform sampler2D texture_diffuse;
uniform sampler2D texture_specular;
uniform sampler2D texture_emissive;

uniform vec3 color_ambient;
uniform vec3 color_diffuse;
uniform vec3 color_specular;
uniform vec3 color_emissive;

uniform float shininess;

in vec3 frag_pos;
in vec3 frag_normal;
in vec2 frag_tcoord;
flat in int frag_mtl;
in vec3 local_light1;
in vec3 local_light2;

out vec4 fragColor;

vec3 ambient() {

  vec3 rgb = color_ambient;
  if (!isnan(frag_tcoord.s) && (0 != (texture_active & 0x01)))
    rgb *= texture(texture_ambient, frag_tcoord).rgb;

  return 0.3 * rgb;
}

vec3 diffuse() {

  vec3 rgb = color_diffuse;
  if (!isnan(frag_tcoord.s) && (0 != (texture_active & 0x02)))
    rgb *= texture(texture_diffuse, frag_tcoord).rgb;

  if (!isnan(frag_normal.x))
    rgb *= 0.3 +
      max(0, 1.0 * dot(frag_normal, local_light1)) +
      max(0, 0.5 * dot(frag_normal, local_light2));

  return rgb;
}

vec3 specular() {

  if (isnan(frag_normal.x))
    return vec3(0);

  vec3 rgb = color_specular;
  if (!isnan(frag_tcoord.s) && (0 != (texture_active & 0x04)))
    rgb *= texture(texture_diffuse, frag_tcoord).rgb;

  vec3 light1_refl = reflect(-local_light1, frag_normal);
  vec3 light2_refl = reflect(-local_light2, frag_normal);

  vec3 view_dir = normalize(-frag_pos);
  float spec1 = pow(max(dot(view_dir, light1_refl), 0.0), shininess);
  float spec2 = pow(max(dot(view_dir, light2_refl), 0.0), shininess);

  return (spec1 + spec2) * rgb;
}

vec3 emission() {

  vec3 rgb = color_emissive;
  if (!isnan(frag_tcoord.s) && (0 != (texture_active & 0x08)))
    rgb *= texture(texture_emissive, frag_tcoord).rgb;

  return rgb;
}

void main() {
  if (frag_mtl < 0) {
    fragColor = vec4(1,0,1,1);
  } else {
    vec3 rgb = min(vec3(1,1,1), ambient() + diffuse() + specular() + emission());
    fragColor = vec4(rgb, 1);
  }
}
)lang=glsl";

void ObjRenderer::Impl::setup() {
  is_setup = true;

  GM_DBG2("ObjRenderer", "Creating vertex shader");
  vertex_shader_id = glCreateShader(GL_VERTEX_SHADER);
  const char *vert_strs[] = { vertex_shader_code.c_str() };
  glShaderSource(vertex_shader_id, 1, vert_strs, nullptr);
  glCompileShader(vertex_shader_id);

  GM_DBG2("ObjRenderer", "Creating fragment shader");
  fragment_shader_id = glCreateShader(GL_FRAGMENT_SHADER);
  const char *frag_strs[] = { fragment_shader_code.c_str() };
  glShaderSource(fragment_shader_id, 1, frag_strs, nullptr);
  glCompileShader(fragment_shader_id);

  GM_DBG2("ObjRenderer", "Creating and linking program");
  program_id = glCreateProgram();
  glAttachShader(program_id, vertex_shader_id);
  glAttachShader(program_id, fragment_shader_id);
  glLinkProgram(program_id);

  if (!GLUtils::check_shader_program(program_id))
    return;

  GM_DBG2("ObjRenderer", "Creating vertex array");
  glGenVertexArrays(1, &vao_id);
  glBindVertexArray(vao_id);

  GM_DBG2("ObjRenderer", "Creating and setting up array buffer");
  glGenBuffers(4, vbo_id);

  glBindBuffer(GL_ARRAY_BUFFER, vbo_id[0]);
  glBufferData(GL_ARRAY_BUFFER,
               vertices.size() * sizeof(GLfloat),
               vertices.data(), GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

  glBindBuffer(GL_ARRAY_BUFFER, vbo_id[1]);
  glBufferData(GL_ARRAY_BUFFER,
               normals.size() * sizeof(GLfloat),
               normals.data(), GL_STATIC_DRAW);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

  glBindBuffer(GL_ARRAY_BUFFER, vbo_id[2]);
  glBufferData(GL_ARRAY_BUFFER,
               tcoords.size() * sizeof(GLfloat),
               tcoords.data(), GL_STATIC_DRAW);
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);

  glBindBuffer(GL_ARRAY_BUFFER, vbo_id[3]);
  glBufferData(GL_ARRAY_BUFFER,
               mtls.size() * sizeof(GLint),
               mtls.data(), GL_STATIC_DRAW);
  glVertexAttribPointer(3, 1, GL_INT, GL_FALSE, 0, 0);

  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);
  glEnableVertexAttribArray(2);

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);

  is_functional = true;
}

namespace {
}

bool ObjRenderer::Impl::read_obj(std::vector<GLfloat> &vertices,
                                 std::vector<GLfloat> &normals,
                                 std::vector<GLfloat> &tcoords,
                                 std::vector<GLint> &mtls) {

  tinyobj::ObjReader objreader;

  try {
    const auto filepath = gmCore::FileResolver::getDefault()->resolve(
        file, gmCore::FileResolver::Check::ReadableFile);
	tex_root_path = filepath.parent_path();

    tinyobj::ObjReaderConfig reader_config;
    reader_config.triangulate = true;
    reader_config.vertex_color = false;
    reader_config.mtl_search_path = filepath.parent_path().u8string();

    if (!objreader.ParseFromFile(filepath.u8string(), reader_config)) {
      const auto err = objreader.Error();
      if (err.empty())
        GM_ERR("ObjRenderer",
               "Unknown error while reading file '" << filepath << "' ('"
                                                    << file << "')");
      else
        GM_ERR("ObjRenderer",
               "Error while reading file '" << filepath << "' ('" << file
                                            << "'): " << err);
      return false;
    }

    std::string wrn = objreader.Warning();
    if (!wrn.empty()) {
      if (wrn.back() == '\n') wrn.pop_back();
      GM_WRN("ObjRenderer",
             "Warning while reading file '" << filepath << "': " << wrn);
    }

  } catch (gmCore::InvalidArgument &err) {
    GM_ERR("ObjRenderer", "Error: " << err.what);
    return false;
  }

  index_map_t index_map;

  const auto &attrib = objreader.GetAttrib();
  const auto &shapes = objreader.GetShapes();
  const auto &materials = objreader.GetMaterials();

  vertices.reserve(attrib.vertices.size());
  normals.reserve(attrib.normals.size());
  tcoords.reserve(attrib.texcoords.size());
  mtls.reserve(attrib.texcoords.size() / 2);

  if (materials.size() > 1)
    GM_WRN("ObjRenderer",
           "Only one material supported (" << materials.size() << " > 1)!");

  for (const auto &material : materials)
    this->materials.push_back(makeMaterial(material));

  for (size_t idx = 0; idx < shapes.size(); ++idx) {

    const auto &shape = shapes[idx];

    if (!shape.points.indices.empty())
      GM_WRN("ObjRenderer", "Skipping points - not supported");

    if (!shape.lines.num_line_vertices.empty())
      GM_WRN("ObjRenderer", "Skipping lines - not supported");

    if (!shape.mesh.num_face_vertices.empty()) {
      addTriangle(vertices,
                  normals,
                  tcoords,
                  mtls,
                  index_map,
                  attrib,
                  shape.mesh);
    }
  }

  return true;
}

void ObjRenderer::Impl::addTriangle(std::vector<GLfloat> &vertices,
                                    std::vector<GLfloat> &normals,
                                    std::vector<GLfloat> &tcoords,
                                    std::vector<GLint> &mtls,
                                    index_map_t &index_map,
                                    const tinyobj::attrib_t &attrib,
                                    const tinyobj::mesh_t &mesh) {

  std::vector<GLuint> shape_indices;
  size_t mesh_indices_offset = 0;

  for (size_t face_idx = 0; face_idx < mesh.num_face_vertices.size();
       ++face_idx) {

    size_t num_face_vertices = mesh.num_face_vertices[face_idx];

    if (num_face_vertices != 3) {
      GM_RUNONCE(GM_WRN(
          "ObjRenderer",
          "Loader could not correctly triangulate: " << num_face_vertices));
      continue;
    }

    for (size_t vert_idx = 0; vert_idx < num_face_vertices; ++vert_idx) {

      const auto &f_i = mesh.indices[mesh_indices_offset + vert_idx];
      const std::array<int, 3> a_i = {
          f_i.vertex_index, f_i.normal_index, f_i.texcoord_index};

      auto pt = index_map.find(a_i);
      if (pt != index_map.end()) {
        shape_indices.push_back(pt->second);
      } else {

        shape_indices.push_back(vertices.size() / 3);
        index_map[a_i] = shape_indices.back();

        vertices.push_back(attrib.vertices[f_i.vertex_index * 3]);
        vertices.push_back(attrib.vertices[f_i.vertex_index * 3 + 1]);
        vertices.push_back(attrib.vertices[f_i.vertex_index * 3 + 2]);

        static_assert(std::numeric_limits<GLfloat>::has_quiet_NaN);

        if (f_i.normal_index >= 0) {
          Eigen::Vector3f normal(attrib.normals[f_i.normal_index * 3],
                                 attrib.normals[f_i.normal_index * 3 + 1],
                                 attrib.normals[f_i.normal_index * 3 + 2]);
          float norm = normal.norm();
          if (std::fabs(norm - 1) > 1e-3) {
            GM_RUNONCE(GM_WRN("ObjRenderer",
                              "Normalizing non-normalized normals (off by "
                                  << std::fabs(norm - 1) << ")"));
            normal /= norm;
          }
          normals.push_back(attrib.normals[f_i.normal_index * 3]);
          normals.push_back(attrib.normals[f_i.normal_index * 3 + 1]);
          normals.push_back(attrib.normals[f_i.normal_index * 3 + 2]);
        } else {
          normals.push_back(std::numeric_limits<GLfloat>::quiet_NaN());
          normals.push_back(std::numeric_limits<GLfloat>::quiet_NaN());
          normals.push_back(std::numeric_limits<GLfloat>::quiet_NaN());
        }

        if (f_i.texcoord_index >= 0) {
          tcoords.push_back(attrib.texcoords[f_i.texcoord_index * 2]);
          tcoords.push_back(attrib.texcoords[f_i.texcoord_index * 2 + 1]);
        } else {
          tcoords.push_back(std::numeric_limits<GLfloat>::quiet_NaN());
          tcoords.push_back(std::numeric_limits<GLfloat>::quiet_NaN());
        }

        if (face_idx < mesh.material_ids.size()) {
          mtls.push_back(face_idx);
        } else {
          mtls.push_back(-1);
        }
      }
    }

    mesh_indices_offset += num_face_vertices;
  }

  triangle_indices.emplace_back(std::move(shape_indices));
}

ObjRenderer::Material
ObjRenderer::Impl::makeMaterial(const tinyobj::material_t &mat) {

  Material res = {{mat.ambient[0], mat.ambient[1], mat.ambient[2]},
                  {mat.diffuse[0], mat.diffuse[1], mat.diffuse[2]},
                  {mat.specular[0], mat.specular[1], mat.specular[2]},
                  {mat.emission[0], mat.emission[1], mat.emission[2]},
                  mat.shininess};

#define LOAD_TEXTURE(XXX)                                                      \
  if (!mat.XXX##_texname.empty()) {                                            \
    auto str = mat.XXX##_texname;                                              \
    auto tmp = std::find_if(                                                   \
        materials.begin(), materials.end(), [&str](const Material &mat) {      \
          return str == mat.texture_##XXX->getFile();                          \
        });                                                                    \
    if (tmp != materials.end()) {                                              \
      res.texture_##XXX = tmp->texture_##XXX;                                  \
    } else {                                                                   \
      auto tex = std::make_shared<ImageTexture>();                             \
      tex->setFile(tex_root_path / str);                                       \
      tex->initialize();                                                       \
      res.texture_##XXX = tex;                                                 \
      if (res.color_##XXX.sum() < std::numeric_limits<float>::epsilon())       \
        GM_WRN("ObjRenderer",                                                  \
               "Texture " << #XXX                                              \
                          << " will not have any effect since "                \
                             "corresponding mesh color is zero.");             \
    }                                                                          \
  }
  LOAD_TEXTURE(ambient);
  LOAD_TEXTURE(diffuse);
  LOAD_TEXTURE(specular);
  LOAD_TEXTURE(emissive);

  GM_DBG2("ObjRenderer",
          "Colors: amb=(" << mat.ambient[0] << ", " << mat.ambient[1] << ", "
                          << mat.ambient[2] << "), diff=(" << mat.diffuse[0]
                          << ", " << mat.diffuse[1] << ", " << mat.diffuse[2]
                          << "), spec=(" << mat.specular[0] << ", "
                          << mat.specular[1] << ", " << mat.specular[2]
                          << "), emis=(" << mat.emission[0] << ", "
                          << mat.emission[1] << ", " << mat.emission[2]
                          << ") and shininess=" << mat.shininess);
  GM_DBG2("ObjRenderer",
          "Textures: amb='"
              << mat.ambient_texname << "', diff='" << mat.diffuse_texname
              << "', spec='" << mat.specular_texname << "', spec='"
              << mat.specular_highlight_texname << "', bump='"
              << mat.bump_texname << "', displac='" << mat.displacement_texname
              << "', alpha='" << mat.alpha_texname << "' and refl='"
              << mat.reflection_texname << "'");

  return res;
}

void ObjRenderer::Impl::render(const Camera &camera, const Eigen::Affine3f &Mm) {

  if (!is_setup)
    setup();
  if (!is_functional)
    return;

  GM_DBG2("ObjRenderer", "rendering");

  Eigen::Affine3f Mv = camera.getViewMatrix();
  Eigen::Matrix3f Mn = Mm.matrix().block<3, 3>(0, 0).inverse().transpose();
  Eigen::Matrix4f Mp = camera.getProjectionMatrix();

#  define LOC(VAR)                                                             \
    (loc[program_id].VAR > 0                                                   \
         ? loc[program_id].VAR                                                 \
         : (loc[program_id].VAR = glGetUniformLocation(program_id, #VAR)))

  glUseProgram(program_id);
  glUniformMatrix4fv(LOC(Mp), 1, false, Mp.data());
  glUniformMatrix4fv(LOC(Mv),  1, false, Mv.data());
  glUniformMatrix3fv(LOC(Mn),  1, false, Mn.data());
  glUniformMatrix4fv(LOC(Mm),  1, false, Mm.data());
  glUniform1i(LOC(texture_ambient), 0);
  glUniform1i(LOC(texture_diffuse), 1);
  glUniform1i(LOC(texture_specular), 2);
  glUniform1i(LOC(texture_emissive), 3);

  GLint texture_active = 0;
  if (!materials.empty()) {
    const Material &mat = materials.front();

    glUniform3fv(LOC(color_ambient), 1, mat.color_ambient.data());
    glUniform3fv(LOC(color_diffuse), 1, mat.color_diffuse.data());
    glUniform3fv(LOC(color_specular), 1, mat.color_specular.data());
    glUniform3fv(LOC(color_emissive), 1, mat.color_emissive.data());
    glUniform1f(LOC(shininess), mat.shininess);

    if (mat.texture_ambient) {
      texture_active |= 0x1;
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D,
                    mat.texture_ambient->updateTexture(camera.frame_number,
                                                       camera.getEye()));
    }
    if (mat.texture_diffuse) {
      texture_active |= 0x1 << 1;
      glActiveTexture(GL_TEXTURE1);
      glBindTexture(GL_TEXTURE_2D,
                    mat.texture_diffuse->updateTexture(camera.frame_number,
                                                       camera.getEye()));
    }
    if (mat.texture_specular) {
      texture_active |= 0x1 << 2;
      glActiveTexture(GL_TEXTURE2);
      glBindTexture(GL_TEXTURE_2D,
                    mat.texture_specular->updateTexture(camera.frame_number,
                                                        camera.getEye()));
    }
    if (mat.texture_emissive) {
      texture_active |= 0x1 << 3;
      glActiveTexture(GL_TEXTURE3);
      glBindTexture(GL_TEXTURE_2D,
                    mat.texture_emissive->updateTexture(camera.frame_number,
                                                        camera.getEye()));
    }
  } else {
    static Eigen::Vector3f black = Eigen::Vector3f::Zero();
    static Eigen::Vector3f gray = Eigen::Vector3f(0.8f, 0.8f, 0.8f);
    glUniform3fv(LOC(color_ambient), 1, black.data());
    glUniform3fv(LOC(color_diffuse), 1, gray.data());
    glUniform3fv(LOC(color_specular), 1, black.data());
    glUniform3fv(LOC(color_emissive), 1, black.data());
    glUniform1f(LOC(shininess), 32);
  }

  glUniform1i(LOC(texture_active), texture_active);

  glBindVertexArray(vao_id);
  for (size_t idx = 0; idx < triangle_indices.size(); ++idx) {
    const auto &ind = triangle_indices[idx];
    glDrawElements(GL_TRIANGLES, ind.size(), GL_UNSIGNED_INT, ind.data());
  }

  for (GLint idx = 0; idx < 4; ++idx) {
    glActiveTexture(GL_TEXTURE0 + idx);
    glBindTexture(GL_TEXTURE_2D, 0);
  }

  glBindVertexArray(0);
  glUseProgram(0);

  GM_DBG3("ObjRenderer", "Done rendering");
}

ObjRenderer::Impl::~Impl() {
  teardown();
}

void ObjRenderer::Impl::teardown() {
  is_functional = false;

  if (program_id) glDeleteProgram(program_id);
  if (vertex_shader_id) glDeleteShader(vertex_shader_id);
  if (fragment_shader_id) glDeleteShader(fragment_shader_id);
  if (vao_id) glDeleteVertexArrays(1, &vao_id);
  if (vbo_id[0]) glDeleteBuffers(4, vbo_id);

  vertex_shader_id = 0;
  fragment_shader_id = 0;
  program_id = 0;
  vao_id = 0;
  vbo_id[0] = 0;

  is_setup = false;
}

void ObjRenderer::setFile(std::filesystem::path file) {
  if (_impl->file == file) return;
  _impl->file = file;
  _impl->teardown();
}

void ObjRenderer::setRecenter(bool on) {
  if (_impl->recenter == on) return;
  _impl->recenter = on;
}

std::vector<ObjRenderer::Material> ObjRenderer::getMaterials() const {
  return _impl->materials;
}

void ObjRenderer::setMaterials(
    const std::vector<ObjRenderer::Material> &materials) {
  if (materials.size() != _impl->materials.size())
    throw gmCore::InvalidArgument("Wrong number of materials");

  _impl->materials = materials;
}

std::vector<float> ObjRenderer::getIntersections(const IntersectionLine &line) {
  return _impl->getIntersections(line);
}

std::vector<float>
ObjRenderer::Impl::getIntersections(const IntersectionLine &line) {
  if (!bounds) return {};
  auto bounds_intersections = bounds->getIntersections(line);
  if (bounds_intersections.empty()) return {};

  // TODO: Check triangle intersection
  return bounds_intersections;
}

END_NAMESPACE_GMGRAPHICS;

#endif
