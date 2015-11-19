#include <logging/control_log.h>
#include <thr_queue/util_queue.h>
#include <aio/aio_tcp.h>

#include <gtest/gtest.h>
#include <random>

using namespace game_engine::aio;
using namespace game_engine;

static bool got_accept_res = false;
static unsigned int entered_read_loop = 0;

TEST(AIOSubsystem, EchoServer)
{
  //game_engine::logging::set_default_for_all(game_engine::logging::policy::disable);
  const uint16_t port = 4000;
  const size_t data_transmit = 1024 * 2; // 1 MB
  aio_buffer sent_data;
  aio_buffer received_data;
  thr_queue::event::promise<void> listening;
  auto server_result = thr_queue::default_par_queue().submit_work([&, port, data_transmit] {
    passive_tcp_socket server;
    server.bind_and_listen(port)->perform().get();
    listening.set_value();


    LOG() << "Called accept->perform()";
    auto accept_res_fut = server.accept()->perform();
    LOG() << "Got accept future";
    auto accept_res = accept_res_fut.get();
    LOG() << "Got incomming connection";
    got_accept_res = true;
    active_tcp_socket client(std::move(accept_res.client_sock));

    size_t left_to_read = data_transmit;
    while (left_to_read > 0) {
      ++entered_read_loop;
      auto rres = client.read(1, left_to_read)->perform().get();
      EXPECT_GE(rres.last_status, 0);
      if (rres.last_status > 0) {
        left_to_read -= rres.already_read;
        rres.buf.len = rres.already_read;
        received_data.append(std::move(rres.buf));
      } else if (rres.last_status < 0) {
        std::ostringstream ss;
        ss << "Error reading: " << uv_strerror(rres.last_status);
        LOG() << ss.str();
        throw aio_runtime_error(ss.str());
      }
    }
    LOG() << received_data.len;
  });
  listening.get_future().wait();
  thr_queue::default_par_queue().submit_work([&, port, data_transmit] {
    active_tcp_socket client;
    sockaddr_storage addr;
    uv_ip4_addr("127.0.0.1", 4000, (sockaddr_in*)&addr);
    LOG() << "Connecting...";
    auto connect_res = client.connect(addr)->perform().get();
    if (!connect_res.success) {
      std::ostringstream ss;
      ss << "Error connecting: " << uv_strerror(connect_res.status);
      LOG() << ss.str();
      throw aio_runtime_error(ss.str());
    }

    LOG() << "Connected OK";
    size_t data_left_to_send = data_transmit;
    std::mt19937_64 rand{ std::random_device()() };
    while (data_left_to_send > 0) {
      aio_buffer send_buffer(1024);
      std::generate_n(send_buffer.base, send_buffer.len, rand);
      sent_data.append(send_buffer);
      client.write(std::move(send_buffer));
      data_left_to_send -= 1024;
    }
  }).wait();
  server_result.wait();

  ASSERT_EQ(data_transmit, sent_data.len);
  ASSERT_EQ(data_transmit, received_data.len);
  
  ASSERT_TRUE(std::equal(sent_data.base, sent_data.base + sent_data.len, received_data.base));
}

TEST(AIOSubsystem, ReallyBlocking) {
#ifdef _WIN32
  auto blocking_op = make_aio_operation([] (perform_helper<void> &help){
    thr_queue::event::promise<void> prom;
    help.set_future(prom.get_future());
    help.about_to_block();
    SleepEx(INFINITE, true);
    Sleep(10);
    help.cant_block_anymore();
    prom.set_value();
  });

  thr_queue::default_par_queue().submit_work([&] {
    auto fut = blocking_op->perform();
    EXPECT_EQ(false, fut.ready());
    fut.wait();
  }).wait();
#endif
}
