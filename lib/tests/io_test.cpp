#include <thr_queue/util_queue.h>
#include <aio/aio_tcp.h>

#include <gtest/gtest.h>

using namespace game_engine::aio;
using namespace game_engine;

TEST(TimeSubsystem, TimeChanges)
{
  return;
  thr_queue::default_par_queue().submit_work([] {
    passive_tcp_socket server;
    auto bind_listen_result_fut = server.bind_and_listen(4000)->perform();
    auto bind_listen_res = bind_listen_result_fut.get();

    while(true) {
      auto accept_res = server.accept()->perform().get();
      thr_queue::default_par_queue().submit_work(
      [accept_res = std::move(accept_res)] {
        active_tcp_socket client(std::move(accept_res.client_sock));

        while (true) {
          auto rres = client.read(1024, 1024 * 1024)->perform().get();
          if (rres.last_status > 0) {
            rres.buf.len = rres.already_read;
            client.write(std::move(rres.buf))->perform();
          } else if (rres.last_status < 0) {
            std::cout << "Error: " << uv_strerror(rres.last_status) << '\n';
            break;
          }
        }
      });
    }
  }).wait();
}

TEST(AIOSubsystem, ReallyBlocking) {
  auto blocking_op = make_aio_operation([] (perform_helper<void> &help){
    thr_queue::event::promise<void> prom;
    help.set_future(prom.get_future());
    help.about_to_block();
#ifdef _WIN32
    SleepEx(INFINITE, true);
    Sleep(10);
#else
    usleep(10 * 1000); // 10 ms
#endif
    help.cant_block_anymore();
    prom.set_value();
  });

  thr_queue::default_par_queue().submit_work([&] {
    auto fut = blocking_op->perform();
    EXPECT_EQ(false, fut.ready());
    fut.wait();
  }).wait();
}
