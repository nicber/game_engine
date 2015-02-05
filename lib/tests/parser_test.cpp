#include <gtest/gtest.h>
#include "opengl/parser/obj_parser.h"

using namespace game_engine::opengl::parser;


TEST(ParserTest, CorrectParsing) {
  const std::string file_to_parse =
  "v 0 0 0" // no newline
  "v 1 0 0\n"
  "   v  1  1 \t 0    \n" // lots of whitespace
  "\nv 0 1 0\n" // double newline
  "vt 0 0" // no newline
  "vt 1 0\n"
  "   vt \t 1  1"
  "\n vt 0 1 1\n" //double newline and also we add the w component
  "\nn 1  2      3\n"
  "\tn 2 1 3\n\n"
  "f 1/1 2/2 3/3" //no newline at the end
  "f 1/1/1 3/3/1 4/4/2"
  "f 1/1 2/2 3/2           "; // trailing whitespace

  auto model = parse_obj_format(file_to_parse);

  for (size_t i(0); i < 3; ++i) {
    EXPECT_EQ(glm::vec3(), model.normals[model.indices[i]]);
    EXPECT_EQ(glm::vec3(), model.normals[model.indices[i + 6]]);
  }

  EXPECT_EQ(glm::vec3(1,2,3), model.normals[model.indices[3]]);
  EXPECT_EQ(glm::vec3(1,2,3), model.normals[model.indices[4]]);
  EXPECT_EQ(glm::vec3(2,1,3), model.normals[model.indices[5]]);

  EXPECT_EQ(glm::vec3(), model.tex_coords[model.indices[0]]);
  EXPECT_EQ(glm::vec3(), model.tex_coords[model.indices[3]]);
  EXPECT_EQ(glm::vec3(), model.tex_coords[model.indices[6]]);

  EXPECT_EQ(glm::vec3(1,0,0), model.tex_coords[model.indices[1]]);
  EXPECT_EQ(glm::vec3(1,0,0), model.tex_coords[model.indices[7]]);
  EXPECT_EQ(glm::vec3(1,0,0), model.tex_coords[model.indices[8]]);

  EXPECT_EQ(glm::vec3(1,1,0), model.tex_coords[model.indices[2]]);
  EXPECT_EQ(glm::vec3(1,1,0), model.tex_coords[model.indices[4]]);

  EXPECT_EQ(glm::vec3(0,1,1), model.tex_coords[model.indices[5]]);
}

TEST(ParserTest, VertOutOfBounds) {
  const std::string file_to_parse =
  "v 0 0 0\n"
  "v 1 1 1\n"
  "vt 1 1\n"
  "f 1/1 2/1 3/1\n";

  EXPECT_THROW(parse_obj_format(file_to_parse), inconsistent_file);
}

TEST(ParserTest, TexCoordOutOfBounds) {
  const std::string file_to_parse =
  "v 0 0 0\n"
  "v 1 1 1\n"
  "vt 1 1\n"
  "vt 0 1\n"
  "f 1/1 2/2 1/3\n";

  EXPECT_THROW(parse_obj_format(file_to_parse), inconsistent_file);
}

TEST(ParserTest, NormalOutOfBounds) {
  const std::string file_to_parse =
  "v 0 0 0\n"
  "v 1 1 1\n"
  "f 1//1 2//1 2//1\n";

  EXPECT_THROW(parse_obj_format(file_to_parse), inconsistent_file);
}

TEST(ParserTest, NormalOutOfBounds2) {
  const std::string file_to_parse =
  "v 0 0 0\n"
  "v 1 1 1\n"
  "n 1 2 3\n"
  "n 2 1 3\n"
  "f 1//2 2//1 1//3\n";

  EXPECT_THROW(parse_obj_format(file_to_parse), inconsistent_file);
}
