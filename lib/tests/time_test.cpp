#include "component.h"
#include "entity.h"
#include "spec_subsystem.h"
#include "subsystems/time.h"
#include "world.h"
#include "gtest/gtest.h"

#include "thr_queue/thread_api.h"

using namespace game_engine::logic;

struct test_comp : public component
{
	game_engine::logic::time previous_time;
	bool first_tick = true;

	void read_time()
	{
		auto& time_sub = get_parent_game().get<subsystems::time_subsystem>();
		auto time_sub_abs = time_sub.absolute();
		auto time_delta = time_sub.us_since_last_tick();

		if (first_tick)
		{
			first_tick = !first_tick;
			EXPECT_EQ(time_delta, game_engine::logic::time(time::type::usec, 0));
		}
		else {
			EXPECT_GT(time_sub_abs, previous_time);
		}

		EXPECT_EQ(time_sub_abs - previous_time, time_delta);

		previous_time = time_sub_abs;
	}
};

class test_ent : public entity
{

public:
	test_ent()
	{
		new_component<test_comp>();
	}

	void read_time_from_component()
	{
		get_comp<test_comp>().read_time();
	}
};

/* Class needed for the component to be added to the game. */
class test_subsystem : public util::specialized_subsystem<test_comp>
{
	void update_all()
	{
		for (auto& comp_vec : reg_components)
		{
			for (auto& comp : comp_vec.second)
			{
				static_cast<test_comp&>(*comp).read_time();
			}
		}
	}
};

TEST(TimeSubsystem, TimeChanges)
{
	game gam;
	gam.new_subsystem<subsystems::time_subsystem>();

	gam.new_subsystem<test_subsystem>();
	auto ent_ptr = std::unique_ptr<entity>(new test_ent);

	gam.add_entity(std::move(ent_ptr));

	for (size_t i = 0; i < 50; ++i)
	{
		gam.tick();
		boost::this_thread::sleep_for(boost::chrono::milliseconds(1));
	}
}