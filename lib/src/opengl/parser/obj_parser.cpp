#include <algorithm>
#include <boost/functional/hash.hpp>
#include <cstdio>
#include <cstring>
#include "opengl/parser/obj_parser.h"
#include <sstream>
#include <unordered_map>

namespace game_engine {
namespace opengl {
namespace parser {
inconsistent_file::inconsistent_file(std::string error_msg_,
                  std::vector<face> faces_,
                  std::vector<glm::vec3> vertices_,
                  std::vector<glm::vec3> tex_coords_,
                  std::vector<glm::vec3> normals_)
 :std::runtime_error(std::move(error_msg_)),
  faces(std::move(faces_)),
  vertices(std::move(vertices_)),
  tex_coords(std::move(tex_coords_)),
  normals(std::move(normals_))
{}

const std::vector<face> &inconsistent_file::get_faces() const {
  return faces;
}

const std::vector<glm::vec3> &inconsistent_file::get_vertices() const {
  return vertices;
}

const std::vector<glm::vec3> &inconsistent_file::get_tex_coords() const {
  return tex_coords;
}

const std::vector<glm::vec3> &inconsistent_file::get_normals() const {
  return normals;
}


static std::string create_message(const std::string &message_str, const std::string &source_str, size_t pos) {
  size_t context_start(pos >= 10 ? pos - 10 : 0);
  std::stringstream ss;

  ss << "parsing failure aproximately at position " << pos << " : " << message_str
     << " in source:\n" << std::string(source_str, context_start);

  return ss.str();
}

parse_error::parse_error(std::string message_str_, std::string source_str_, size_t pos_)
 :std::runtime_error(create_message(message_str_, source_str_, pos_)),
  message_str(std::move(message_str_)),
  source_str(std::move(source_str_)),
  pos(pos_)
{}

const std::string &parse_error::error_message() const {
  return message_str;
}

const std::string &parse_error::source_string() const {
  return source_str;
}

size_t parse_error::position() const {
  return pos;
}

static size_t hash_value(const v_vt_n_indices &i) {
  size_t seed(0);
  boost::hash_combine(seed, i.v_i);
  boost::hash_combine(seed, i.vt_i);
  boost::hash_combine(seed, i.n_i);
  return seed;
}

static bool operator==(const v_vt_n_indices &lhs, const v_vt_n_indices &rhs) {
  bool r = lhs.v_i == rhs.v_i &&
           lhs.vt_i == rhs.vt_i &&
           lhs.n_i == rhs.n_i;
  if (r) {
    assert(hash_value(lhs) == hash_value(rhs));
  }

  return r;
}

static obj_model create_indices(std::vector<face> faces,
                                std::vector<glm::vec3> vertices,
                                std::vector<glm::vec3> tex_coords,
                                std::vector<glm::vec3> normals)
{
  unsigned int id(1);
  std::vector<unsigned int> new_indices;
  std::vector<glm::vec3> new_vertices;
  std::vector<glm::vec3> new_tex_coords;
  std::vector<glm::vec3> new_normals;
  std::unordered_map<v_vt_n_indices, unsigned int, boost::hash<v_vt_n_indices>> indices_map;

  new_indices.reserve(faces.size() * 3);
  auto move_and_throw_inconsistent_file = [&] (std::string error_msg_) {
    throw inconsistent_file(std::move(error_msg_),
                            std::move(faces),
                            std::move(vertices),
                            std::move(tex_coords),
                            std::move(normals));
  };

  for (auto &f : faces) {
    for (auto &ind : f.indices) {
      if (ind.v_i > vertices.size()) {
        std::stringstream ss;
        ss << "vertex index " << ind.v_i << " out of range [1;" << vertices.size() << "]";
        move_and_throw_inconsistent_file(ss.str());
      }
      if (ind.vt_i > tex_coords.size()) {
        std::stringstream ss;
        ss << "texture coords index " << ind.vt_i << " out of range [1;" << tex_coords.size() << "]";
        move_and_throw_inconsistent_file(ss.str());
      }
      if (ind.n_i > normals.size()) {
        std::stringstream ss;
        ss << "normal index " << ind.n_i << " out of range [1;" << normals.size() << "]";
        move_and_throw_inconsistent_file(ss.str());
      }

      // we have to count from 1 so that we can tell which indices are new (i.e. == 0),
      // and which ones have already been processed.
      auto &new_index = indices_map[ind];
      if (new_index == 0) {
        assert(new_vertices.size() == id - 1);
        assert(new_tex_coords.size() == id - 1);
        assert(new_normals.size() == id - 1);
        new_index = id;
        ++id;
        auto tex_coord_new = ind.vt_i ? tex_coords[ind.vt_i - 1] : glm::vec3();
        auto normal_new = ind.n_i ? normals[ind.n_i - 1] : glm::vec3();
        new_vertices.emplace_back(vertices[ind.v_i - 1]);
        new_tex_coords.emplace_back(std::move(tex_coord_new));
        new_normals.emplace_back(std::move(normal_new));
      }
      new_indices.emplace_back(new_index - 1);
    }
  }

  assert (new_indices.size() == faces.size() * 3);

  return {std::move(new_indices), std::move(new_vertices),
          std::move(new_tex_coords), std::move(new_normals)};
}

obj_model parse_obj_format(const char *str) {
  std::vector<face> faces;
  std::vector<glm::vec3> vertices;
  std::vector<glm::vec3> tex_coords;
  std::vector<glm::vec3> normals;
  size_t pos(0);

  while (*str) {
    char def_type[3] = {0};
    int delta_pos;
    bool check_vt(true), check_n(true);

    if (std::sscanf(str + pos, "%2s%n", def_type, &delta_pos) != 1) {
      break;
    }
    pos += delta_pos;

    if (std::strcmp("f", def_type) == 0) {
      --pos; // we reread the f for the sake of clarity.
      unsigned int v_i[3],
                   vt_i[3],
                   n_i[3];

      if (std::sscanf(str + pos, "f %u/%u/%u %u/%u/%u %u/%u/%u%n",
                      &v_i[0], &vt_i[0], &n_i[0],
                      &v_i[1], &vt_i[1], &n_i[1],
                      &v_i[2], &vt_i[2], &n_i[2], &delta_pos) == 9) {
      } else if (std::sscanf(str + pos, "f %u/%u %u/%u %u/%u%n",
                             &v_i[0], &vt_i[0],
                             &v_i[1], &vt_i[1],
                             &v_i[2], &vt_i[2], &delta_pos) == 6) {
        std::fill(std::begin(n_i), std::end(n_i), 0);
        check_n = false;
      } else if (std::sscanf(str + pos, "f %u//%u %u//%u %u//%u%n",
                             &v_i[0], &n_i[0],
                             &v_i[1], &n_i[1],
                             &v_i[2], &n_i[2], &delta_pos) == 6) {
        std::fill(std::begin(vt_i), std::end(vt_i), 0);
        check_vt = false;
      } else {
        throw parse_error("couldn't parse face", str, pos);
      }

      auto check_equal_zero = [](unsigned int i) { return i == 0; };
      bool any_zero_indices(std::any_of(v_i, v_i + 3, check_equal_zero));
      any_zero_indices = any_zero_indices || (check_vt && std::any_of(vt_i, vt_i + 3, check_equal_zero));
      any_zero_indices = any_zero_indices || (check_n && std::any_of(n_i, n_i + 3, check_equal_zero));

      if (any_zero_indices) {
        throw parse_error("0-based indices not allowed in .obj format", str, pos);
      }

      faces.emplace_back(face{{v_vt_n_indices{v_i[0], vt_i[0], n_i[0]},
                              v_vt_n_indices{v_i[1], vt_i[1], n_i[1]},
                              v_vt_n_indices{v_i[2], vt_i[2], n_i[2]}}});
      pos += delta_pos;
    } else if (std::strcmp("v", def_type) == 0) {
      --pos;
      glm::vec3 v;
      float w;
      auto matches = std::sscanf(str + pos, "v %f %f %f%n %f", &v.x, &v.y, &v.z, &delta_pos, &w);

      if (matches == 4) {
        throw parse_error("vertices with w components are not supported", str, pos);
      } else if (matches == 3) {
        vertices.emplace_back(std::move(v));
        pos += delta_pos;
      } else {
        throw parse_error("format: v %f %f %f%n %f", str, pos);
      }
    } else if (std::strcmp("vt", def_type) == 0) {
      pos -= 2;
      int delta_pos_with_z, delta_pos_without_z;
      glm::vec3 vt;
      auto matches =std::sscanf(str + pos, "vt %f %f%n %f%n", &vt.x, &vt.y, &delta_pos_without_z,
                                &vt.z, &delta_pos_with_z);

      if (matches == 3) {
        pos += delta_pos_with_z;
        tex_coords.emplace_back(std::move(vt));
      } else if (matches == 2) {
        pos += delta_pos_without_z;
        tex_coords.emplace_back(std::move(vt));
      } else {
        throw parse_error("format: vt %f %f%n %f%n", str, pos);
      }
    } else if (std::strcmp("n", def_type) == 0) {
      --pos;
      glm::vec3 n;
      if (std::sscanf(str + pos, "n %f %f %f%n", &n.x, &n.y, &n.z, &delta_pos) == 3) {
        pos += delta_pos;
        normals.emplace_back(std::move(n));
      } else {
        throw parse_error("format: n %f %f %f%n", str, pos);
      }
    } else if (def_type[0] == '#') {
      //comment
      std::sscanf(str + pos, "%*[^\n]%n", &delta_pos);
      pos += delta_pos;
    } else {
      throw parse_error("unknown definition type", str, pos);
    }
  }

  return create_indices(faces, vertices, tex_coords, normals);
}

obj_model parse_obj_format(const std::string &str) {
  return parse_obj_format(str.c_str());
}
}
}
}
