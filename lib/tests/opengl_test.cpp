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

TEST(OpenGLTest, UniformBindingBlock) {
  sf::Context ctx;
  glewInit();
  std::string name = "a";
  std::vector<std::shared_ptr<const uniform_block_binding>> already_allocated_blocks;

  try {
    while (true) {
      already_allocated_blocks.emplace_back(get_free_uniform_block_binding(name));
      name += "a";
    }
  } catch (std::runtime_error &e) {
  }

  for (auto & ptr : already_allocated_blocks) {
    EXPECT_NO_THROW(get_free_uniform_block_binding(ptr->get_name()));
  }
  name += "a";
  EXPECT_THROW(get_free_uniform_block_binding(name), std::runtime_error);
  auto max_allocated = already_allocated_blocks.size();
  already_allocated_blocks.clear();
  name = "a";
  for (size_t i = 0; i < max_allocated; ++i) {
    EXPECT_NO_THROW({already_allocated_blocks.emplace_back(get_free_uniform_block_binding(name));});
    name += "a";
  }
  EXPECT_THROW(get_free_uniform_block_binding(name), std::runtime_error);
}
