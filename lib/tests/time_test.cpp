#include "entity.h"
#include "spec_subsystem.h"
#include "subsystems/time.h"
#include "world.h"
#include "gtest/gtest.h"

#include <thread>

using namespace game_engine::logic;

class test_ent : public entity
{
	game_engine::logic::time previous_time;
	bool first_tick = true;
public:
	void read_time_from_entity()
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

/* Class needed for the entity to be added to the game. */
class test_subsystem : public util::specialized_subsystem<test_ent>
{
	void update_all() {}
};

TEST(TimeSubsystem, TimeChanges)
{
	game gam;
	gam.new_subsystem<subsystems::time_subsystem>();

	gam.new_subsystem<test_subsystem>();
	auto ent_ptr = std::unique_ptr<entity>(new test_ent);
	auto& test_ent_ref = static_cast<test_ent&>(*ent_ptr);

	gam.add_entity(std::move(ent_ptr));

	for (size_t i = 0; i < 50; ++i)
	{
		gam.tick();
		test_ent_ref.read_time_from_entity();
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
}