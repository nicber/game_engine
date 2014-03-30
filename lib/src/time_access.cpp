#include "time_access.h"

#include <cstdio>
#include <iostream>
#include <limits>
#include <memory>

namespace game_engine
{
	namespace logic
	{
		time operator+(time lhs, const time& rhs)
		{
			return lhs += rhs;
		}
		time operator-(time lhs, const time& rhs)
		{
			return lhs -= rhs;
		}

		time& operator+=(time& lhs, const time& rhs)
		{
			lhs.plat_time += rhs.plat_time;
			return lhs;
		}
		time& operator-=(time& lhs, const time& rhs)
		{
			lhs.plat_time -= rhs.plat_time;
			return lhs;
		}

		bool operator<(const time& lhs, const time& rhs) { return lhs.plat_time < rhs.plat_time; }
		bool operator>(const time& lhs, const time& rhs) { return lhs.plat_time > rhs.plat_time; }
		bool operator<=(const time& lhs, const time& rhs) { return lhs.plat_time <= rhs.plat_time; }
		bool operator>=(const time& lhs, const time& rhs) { return lhs.plat_time >= rhs.plat_time; }
		bool operator==(const time& lhs, const time& rhs) { return lhs.plat_time == rhs.plat_time; }
		bool operator!=(const time& lhs, const time& rhs) { return lhs.plat_time != rhs.plat_time; }

		time::time(type typ, long long amount)
		{
			using namespace platform;
			switch (typ)
			{
			case type::platform:
				plat_time = amount;
				break;
			case type::usec:
				plat_time = from_us(amount);
				break;
			case type::msec:
				plat_time = from_us(amount * 1000);
				break;
			case type::sec:
				plat_time = from_us(amount * 1000 * 1000);
				break;
			}
		}

		std::ostream& operator<<(std::ostream& ostr, const time& tim)
		{
			ostr << tim.to(time::type::usec) << " microseconds";
			return ostr;
		}

		long long time::to(type typ) const
		{
			if (typ == type::platform)
			{
				return plat_time;
			}

			auto res = platform::to_us(plat_time);
			switch (typ)
			{
			case type::usec:
				break;
			case type::msec:
				res /= 1000;
				break;
			case type::sec:
				res /= (1000 * 1000);
				break;
			}

			return res;
		}
	}
}