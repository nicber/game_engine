#include <thr_queue/util_queue.h>
#include <aio/aio_tcp.h>
#include <random>

using namespace game_engine::aio;
using namespace game_engine;

int main(int argc, const char *[])
{
  const uint16_t port = 4000;
  const size_t data_transmit = 1024 * 2; // 1 MB
  unsigned int iteration = 0;

  if (argc == 1) {
    while (true) {
      thr_queue::default_par_queue().submit_work([&, port, data_transmit] {
      {
        LOG() << "Started server";
        passive_tcp_socket server;
        LOG() << "Initialized server";
        server.bind_and_listen(port + iteration % 2)->perform().get();
        LOG() << "Called accept->perform() on " << port + iteration % 2;
        auto accept_res_fut = server.accept()->perform();
        LOG() << "Got accept future";
        auto accept_res = accept_res_fut.get();
        LOG() << "Got incomming connection";
        active_tcp_socket client(std::move(accept_res.client_sock));

        size_t left_to_read = data_transmit;
        while (left_to_read > 0) {
          auto rres = client.read(1, left_to_read)->perform().get();
          assert(rres.last_status > 0);
          if (rres.last_status > 0) {
            left_to_read -= rres.already_read;
            LOG() << "Read " << rres.already_read;
          } else if (rres.last_status < 0) {
            std::ostringstream ss;
            ss << "Error reading: " << uv_strerror(rres.last_status);
            LOG() << ss.str();
            throw aio_runtime_error(ss.str());
          }
        }
      }
      LOG() << "Exiting server";
      ++iteration;
    }).wait();
    }
  } else {
    while (true) {
      thr_queue::default_par_queue().submit_work([&, port, data_transmit] {
        LOG() << "Started client";
        active_tcp_socket client;
        sockaddr_storage addr;
        uv_ip4_addr("127.0.0.1", port + iteration % 2, (sockaddr_in*)&addr);
        LOG() << "Connecting to " << port + iteration % 2 << "...";
        bool successful_connection = false;
        while (!successful_connection) {
          auto connect_res = client.connect(addr)->perform().get();
          if (!connect_res.success) {
            std::ostringstream ss;
            ss << "Error connecting: " << uv_strerror(connect_res.status);
            LOG() << ss.str();
            if(connect_res.status == UV_ECONNREFUSED) {
              return;
            } else {
              throw aio_runtime_error(ss.str());
            }
          }
          successful_connection = true;
        }

        LOG() << "Connected OK";
        size_t data_left_to_send = data_transmit;
        std::mt19937_64 rand{ std::random_device()() };
        while (data_left_to_send > 0) {
          aio_buffer send_buffer(1024);
          std::generate_n(send_buffer.base, send_buffer.len, rand);
          LOG() << "Sending";
          client.write(std::move(send_buffer));
          LOG() << "Sent";
          data_left_to_send -= 1024;
        }
        ++iteration;
      }).wait();
    }
  }
}
