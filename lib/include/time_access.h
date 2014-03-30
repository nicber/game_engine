#pragma once

#include "platform/time.h"

#include <iosfwd>

namespace game_engine
{
	namespace logic
	{
		class time
		{
			/** \brief Stores time in a platform specific type. */
			platform::time_type plat_time = 0;
		public:
			/** \brief Time formats that time can be constructed from and
			 * converted to.
			 */
			enum class type
			{
				platform, usec, msec, sec
			};

			/** \brief Sets the time to 0. */
			time() = default;

			/** \brief Takes time in a format specified in time::type and a certain amount of time.
			 * Performs any necessary conversions between types.
			 */
			time(type typ, long long amount);

			//Arithmetic

			friend time operator+(time lhs, const time& rhs);
			friend time operator-(time lhs, const time& rhs);

			template <typename T>
			friend time operator*(time lhs, const T& rhs);

			template <typename T>
			friend time operator/(time lhs, const T& rhs);

			friend time& operator+=(time& lhs, const time& rhs);
			friend time& operator-=(time& lhs, const time& rhs);

			template <typename T>
			friend time& operator*=(time& lhs, const T& rhs);

			template <typename T>
			friend time& operator/=(time& lhs, const T& rhs);

			//Comp
			friend bool operator<(const time& lhs, const time& rhs);
			friend bool operator>(const time& lhs, const time& rhs);
			friend bool operator<=(const time& lhs, const time& rhs);
			friend bool operator>=(const time& lhs, const time& rhs);
			friend bool operator==(const time& lhs, const time& rhs);
			friend bool operator!=(const time& lhs, const time& rhs);

			friend std::ostream& operator<<(std::ostream& ostr, const time& tim);

			long long to(type typ) const;
		};		

		/** \brief Returns an OS-specific value that represents a point in time
		 * in an implementation defined unit. This clock is monotonic, that is, as physical time
		 * goes forward, values returned by this function should always be
		 * greater or equal to the previous values returned.
		 */
		time read_absolute_time();
	}
}

#include "time_access.inl"