#include <aio/aio_tcp.h>
#include <aio/aio_file.h>
#include <cstring>
#include <boost/filesystem.hpp>
#include <gtest/gtest.h>
#include <logging/control_log.h>
#include <random>
#include <thr_queue/util_queue.h>

using namespace game_engine::aio;
using namespace game_engine;

TEST(AIOSubsystem, EchoServer)
{
  const uint16_t port = 4000;
  const size_t data_transmit = 1024 * 2; // 1 MB
  aio_buffer sent_data;
  aio_buffer received_data;
  thr_queue::event::promise<void> listening;
  auto server_result = thr_queue::default_par_queue().submit_work([&, port, data_transmit] {
    {
      LOG() << "Started server";
      passive_tcp_socket server;
      LOG() << "Initialized server";
      server.bind_and_listen(port)->perform().get();
      listening.set_value();
      LOG() << "Called accept->perform()";
      auto accept_res_fut = server.accept()->perform();
      LOG() << "Got accept future";
      auto accept_res = accept_res_fut.get();
      LOG() << "Got incomming connection";
      active_tcp_socket client(std::move(accept_res.client_sock));
      size_t left_to_read = data_transmit;
      while (left_to_read > 0) {
        auto rres = client.read(1, left_to_read)->perform().get();
        EXPECT_GE(rres.last_status, 0);
        if (rres.last_status > 0) {
          left_to_read -= rres.already_read;
          rres.buf.len = rres.already_read;
          received_data.append(std::move(rres.buf));
        } else if (rres.last_status < 0) {
          throw aio_runtime_error(rres.last_status, "Error reading");
        }
      }
    }
    LOG() << "Exiting server";
  });
  listening.get_future().wait();
  thr_queue::default_par_queue().submit_work([&, port, data_transmit] {
    LOG() << "Started client";
    active_tcp_socket client;
    sockaddr_storage addr;
    uv_ip4_addr("127.0.0.1", 4000, (sockaddr_in*)&addr);
    LOG() << "Connecting...";
    auto connect_res = client.connect(addr)->perform().get();
    if (!connect_res.success) {
      throw aio_runtime_error(connect_res.status, "Error connecting");
    }

    LOG() << "Connected OK";
    size_t data_left_to_send = data_transmit;
    std::mt19937_64 rand{ std::random_device()() };
    while (data_left_to_send > 0) {
      aio_buffer send_buffer(1024);
      std::generate_n(send_buffer.base, send_buffer.len, rand);
      sent_data.append(send_buffer);
      LOG() << "Sending";
      client.write(std::move(send_buffer));
      LOG() << "Sent";
      data_left_to_send -= 1024;
    }
  }).wait();
  server_result.wait();

  ASSERT_EQ(data_transmit, sent_data.len);
  ASSERT_EQ(data_transmit, received_data.len);

  ASSERT_TRUE(std::equal(sent_data.base, sent_data.base + sent_data.len, received_data.base));
}

TEST(AIOSubsystem, ReallyBlocking) {
  auto blocking_op = make_aio_operation([] (perform_helper<void> &help){
    thr_queue::event::promise<void> prom;
    LOG() << "Setting fut_fut " << boost::this_thread::get_id();
    help.set_future(prom.get_future());
    help.about_to_block();
#ifdef _WIN32
    SleepEx(INFINITE, true);
    Sleep(10);
#else
    LOG() << "Sleeping";
    usleep(10000);
    LOG() << "Slept";
#endif
    help.cant_block_anymore();
    LOG() << "Setting value from thread:  " << boost::this_thread::get_id();
    prom.set_value();
  });

  thr_queue::default_par_queue().submit_work([&] {
    auto fut = blocking_op->perform();
    LOG() << "Querying state from thread: " << boost::this_thread::get_id();
    auto fut_state = fut.ready();
    EXPECT_EQ(false, fut_state);
    fut.wait();
  }).wait();
}

TEST(AIOSubsystem, OpenReadFile) {
  auto tmp_dir = boost::filesystem::temp_directory_path();
  auto file_path = tmp_dir.append("file");
  std::ofstream file_stream(file_path.c_str());
  file_stream << "hello world!";
  file_stream.close();

  thr_queue::default_par_queue().submit_work([&] {
    auto aio_file = aio::open(file_path, file_access::read_only,
        file_mode::open_existing)->perform().get();
    auto read_res = aio::read(aio_file, 100, 0)->perform().get();
    ASSERT_EQ(12, read_res.read_total);
    std::string read_string(read_res.buf.base, 12);
    ASSERT_EQ("hello world!", read_string);
    aio::close(aio_file)->perform().wait();
  }).wait();
}

TEST(AIOSubsystem, OpenWriteFile) {
  auto tmp_dir = boost::filesystem::temp_directory_path();
  auto file_path = tmp_dir.append("file");

  thr_queue::default_par_queue().submit_work([&] {
    auto aio_file = aio::open(file_path, file_access::write_only,
        file_mode::create_or_truncate)->perform().get();
    aio_buffer buf(12);
    strncpy(buf.base, "hello world!", buf.len);
    auto write_res = aio::write(aio_file, std::move(buf), 0)->perform().get();
    ASSERT_EQ(12, write_res.total_written);
    aio::close(aio_file)->perform().wait();
  }).wait();

  std::string read_string;
  std::ifstream file_stream(file_path.c_str());
  std::getline(file_stream, read_string);
  ASSERT_EQ("hello world!", read_string);
}

TEST(AIOSubsystem, OpenTruncateFile) {
  auto tmp_dir = boost::filesystem::temp_directory_path();
  auto file_path = tmp_dir.append("file");
  std::ofstream file_stream(file_path.c_str());
  file_stream << "hello world!";
  file_stream.close();

  thr_queue::default_par_queue().submit_work([&] {
    auto aio_file = aio::open(file_path, file_access::write_only,
        file_mode::open_existing)->perform().get();
    aio::truncate(aio_file, 5)->perform().wait();
    aio::close(aio_file)->perform().wait();
  }).wait();

  std::string read_string;
  std::ifstream file_stream2(file_path.c_str());
  std::getline(file_stream2, read_string);
  ASSERT_EQ("hello", read_string);
}
