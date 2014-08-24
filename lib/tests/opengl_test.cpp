#include <gtest/gtest.h>
#include "opengl/buffer.h"
#include "opengl/program.h"
#include "opengl/shader.h"
#include <SFML/Window/Context.hpp>

using namespace game_engine::opengl;

TEST(OpenGLTest, BufferStoreRead) {
  sf::Context ctx;
  glewInit();

  size_t buffer_size = 1024;

  buffer<unsigned int> buff(buffer_size, buf_freq_access::mod_freq, buf_kind_access::read_app_write_gl);

  auto it = buff.begin();
  unsigned int i = 0;

  for (; it != buff.end(); ++it, ++i) {
    *it = i;
  }
  
  EXPECT_EQ(i, buffer_size);
  i = 0;

  for (auto& read_result : buff) {
    unsigned int_result = read_result;
    EXPECT_EQ(int_result, i);
    ++i;
  }
}

TEST(OpenGLTest, BufferResize) {
  sf::Context ctx;
  glewInit();

  size_t buffer_size = 1024;

  buffer<unsigned int> buff(buffer_size, buf_freq_access::mod_freq, buf_kind_access::read_app_write_gl);

  auto it = buff.begin();
  unsigned int i = 0;

  for (; it != buff.end(); ++it, ++i) {
    *it = i;
  }
  
  EXPECT_EQ(i, buffer_size);
  i = 0;

  buff.resize(buffer_size * 2);

  for (auto& read_result : buff) {
    unsigned int_result = read_result;
    if (i < buffer_size) {
      EXPECT_EQ(int_result, i);
    }
    ++i;
  }

  EXPECT_EQ(i, buffer_size * 2);
}

TEST(OpenGLTest, BufferMove) {
  sf::Context ctx;
  glewInit();

  size_t buffer_size = 1024;

  buffer<unsigned int> buff(buffer_size, buf_freq_access::mod_freq, buf_kind_access::read_app_write_gl);

  auto it = buff.begin();
  unsigned int i = 0;

  for (; it != buff.end(); ++it, ++i) {
    *it = i;
  }

  EXPECT_EQ(i, buffer_size);
  i = 0;

  buffer<unsigned int> second_buff(std::move(buff));

  for (auto& read_result : second_buff) {
    unsigned int_result = read_result;
    EXPECT_EQ(int_result, i);
    ++i;
  }

  EXPECT_EQ(i, buffer_size);
}
