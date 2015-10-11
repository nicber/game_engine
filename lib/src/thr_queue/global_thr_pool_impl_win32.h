#pragma once

namespace game_engine {
namespace thr_queue {
namespace platform {
struct work_data : generic_work_data {
  HANDLE iocp;
  const ULONG queue_completionkey = 0x1;
};

class worker_thread_impl : public virtual base_worker_thread {
public:
  worker_thread_impl(work_data &dat);

protected:
  void loop() override;
  void handle_io_operation(OVERLAPPED_ENTRY olapped_entry);
  void please_die() override;
  generic_work_data &get_data() override;

private:
  work_data &data;
};
}
}
}