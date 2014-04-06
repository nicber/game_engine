#include "component.h"
#include "entity.h"
#include "spec_subsystem.h"
#include "subsystems/ai/ai.h"
#include "subsystems/time.h"
#include "world.h"
#include "gtest/gtest.h"

#include <thread>

using namespace game_engine::logic;

struct test_comp1 : public component
{};

struct test_comp2 : public component
{
	int update_n = 0;
};

struct test_comp2_subsys : public util::specialized_subsystem<test_comp2>
{
	void update_all() override
	{}
};

struct test_ai_comp : public subsystems::ai::ai_component
{
	void add_and_remove()
	{
		auto& ent = get_parent_ent();
		std::unique_ptr<component> comp{ new test_comp1 };
		ent.add_component(std::move(comp));
		EXPECT_THROW(ent.new_component<test_comp1>(), entity::component_exists);

		EXPECT_NO_THROW(ent.get_comp<test_comp1>());
		EXPECT_NO_THROW(ent.remove_component<test_comp1>());

		EXPECT_THROW(ent.get_comp<test_comp1>(), entity::component_not_found<test_comp1>);
		EXPECT_THROW(ent.remove_component<test_comp1>(), entity::component_not_found<test_comp1>);
	}

	void check_subsystem()
	{
		static int i = 0;
		static subsystem* previous_ptr = nullptr;
		subsystem* ptr_now;
		switch (i % 4)
		{
		case 0: // No comp, subsystem yes.
			EXPECT_NO_THROW(get_parent_ent().new_component<test_comp2>());
			ptr_now = get_parent_ent().get_comp<test_comp2>().try_get_subsys();
			EXPECT_NE(previous_ptr, ptr_now);
			previous_ptr = ptr_now;
			break;
		case 1: // Comp yes, no subsystem.
			ptr_now = get_parent_ent().get_comp<test_comp2>().try_get_subsys();
			EXPECT_NE(previous_ptr, ptr_now);
			EXPECT_EQ(nullptr, ptr_now);
			previous_ptr = ptr_now;
			break;
		case 2: // Comp yes, subsystem yes
			ptr_now = get_parent_ent().get_comp<test_comp2>().try_get_subsys();
			EXPECT_EQ(previous_ptr, ptr_now);
			EXPECT_EQ(nullptr, ptr_now);
			get_parent_ent().remove_component<test_comp2>();
			break;
		case 4: // No comp, no subsystem
			break;
		}
		i++;
	}

	void ai_update() override
	{
		add_and_remove();
		check_subsystem();
	}
};

class test_ent : public entity
{

public:
	test_ent()
	{
		new_component<test_ai_comp>();
	}
};


TEST(CompEntSubsystem, AddRemove)
{
	game gam;
	gam.new_subsystem<subsystems::ai::ai_subsystem>();
	auto ent_ptr = std::unique_ptr<entity>(new test_ent);

	gam.add_entity(std::move(ent_ptr));

	for (size_t i = 0; i < 50; ++i)
	{
		if (i % 2 == 0)
		{
			gam.new_subsystem<test_comp2_subsys>();
		} else {
			EXPECT_NO_THROW(gam.remove_subsystem<test_comp2_subsys>());
			EXPECT_THROW(gam.get<test_comp2_subsys>(), game::no_subsystem_found<test_comp2_subsys>);
		}
		gam.tick();
	}
}