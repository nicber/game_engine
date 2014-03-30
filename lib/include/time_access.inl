#pragma once

namespace game_engine
{
	namespace logic
	{
		template <typename T>
		time operator*(time lhs, const T& rhs)
		{
			return lhs *= rhs;
		}

		template <typename T>
		time operator/(time lhs, const T& rhs)
		{
			return lhs /= rhs;
		}

		template <typename T>
		time& operator*=(time& lhs, const T& rhs)
		{
			lhs.plat_time *= rhs;
			return lhs;
		}

		template <typename T>
		time& operator/=(time& lhs, const T& rhs)
		{
			lhs.plat_time /= rhs;
			return lhs;
		}
	}
}